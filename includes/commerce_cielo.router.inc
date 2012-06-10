<?php
/**
 * @file
 * Concludes the payment process after returning from Cielo.
 */

/**
 * Callback Function for checkout/%commerce_order/payment/cielo.
 *
 * This page and function are trigged only when there is a redirection back
 * from Cielo.
 */
function commerce_cielo_redirect_back($order) {

  // Set authenticate default value.
  $authenticate = TRUE;

  // Load payment transactions that belongs to this order.
  foreach (commerce_payment_transaction_load_multiple(array(), array('order_id' => $order->order_id)) as $transaction) {
    if ($transaction->remote_status == COMMERCE_CIELO_STATUS_REDIRECTED && $order->status == 'checkout_review') {
      // Load the payment method settings.
      $payment_method = commerce_payment_method_instance_load($transaction->instance_id);
      // Set auto capture.
      $auto_capture = $payment_method['settings']['auto_capture'];

      // Prepare the arguments to pass to object instantiantion of the
      // external library class.
      $arguments = _commerce_cielo_get_arguments($payment_method, $order->order_number, $transaction->amount);

      // Instantiate a object from the external library.
      $cielo = new CommerceCieloLib($arguments);

      // Set the tid assigned by Cielo.
      $cielo->setTid($transaction->remote_id);

      // Only for testing enviroment.
      if ($payment_method['settings']['is_test']) {
        // Make sure we use the same credentials previously used in the
        // checkout.
        $cielo->membership['filiacao'] = $_COOKIE['filiacao'];
        $cielo->membership['chave'] = $_COOKIE['chave'];
      }

      // Look up on the transaction created while away at Cielo.
      $cielo->followUp();

      // Check for and save any warnings that might have been issued by the
      // external library (Cielo PHP Library).
      commerce_cielo_log_watch_dog($cielo);

      $message = '';
      if (isset($cielo->response['status'])) {
        // No transaction errors have occured.
        $status = $cielo->response['status'];
        $message = t('Status Code: @code (Follow Up)', array('@code' => $status));

        foreach ($cielo->response as $process => $values) {
          if (is_array($values)) {
            // Payment tab message.
            if (key_exists('mensagem', $values)) {
              // Assemble payment tab message.
              $message .= '<br />' . t("@process: @message", array('@process' => $process, '@message' => $values['mensagem']));
            }
          }
        }
      }

      $authenticate = !isset($payment_method['settings']['authenticate']) ? TRUE : $payment_method['settings']['authenticate'];

      $remote_status = _commerce_cielo_get_remote_status($cielo, $auto_capture);

      if (empty($remote_status)) {
        $remote_status = _commerce_cielo_get_status_state($authenticate, $status);
      }

      // Prepare a transaction object to log the Webservice response.
      $trans = commerce_payment_transaction_new('cielo', $order->order_id);
      $trans->instance_id = $payment_method['instance_id'];
      $trans->amount = $transaction->amount;
      $trans->currency_code = $transaction->currency_code;
      $trans->remote_id = $transaction->remote_id;
      $trans->remote_status = $remote_status;
      $trans->status = _commerce_cielo_get_status_state($authenticate, $status);
      $trans->message = $message;
      $trans->payload[REQUEST_TIME] = $cielo->response;

      // Save the follow up transaction.
      commerce_payment_transaction_save($trans);

      // Rebuilds the checkout review form if follow up transaction fails.
      if ($trans->status == COMMERCE_PAYMENT_STATUS_FAILURE) {
        drupal_set_message(t('We received the following error processing your card. Please enter you information again or try a different card.'), 'error');
        drupal_set_message($trans->message, 'error');

        // Redirect buyer to the checkout form.
        drupal_goto('checkout/' . $order->order_id . '/review');
      }

      // Capturing.
      if (isset($cielo->response['status']) &&
          $cielo->response['status'] == CommerceCieloLib::AUTHORIZED &&
          $auto_capture == 'true') {
        // Authorization has been granted and auto capture is switched on, so
        // we will try to capture.
        $cielo->capture();

        // Check for and save any warnings that might have been issued by the
        // external library (Cielo PHP Library).
        commerce_cielo_log_watch_dog($cielo);

        // Save the capturing transaction.
        commerce_cielo_save_transaction($cielo, $trans, $authenticate);

        // Rebuilds the checkout review form if follow up transaction fails.
        if ($trans->status == COMMERCE_PAYMENT_STATUS_FAILURE) {
          drupal_set_message(t('We received the following error processing your card. Please enter you information again or try a different card.'), 'error');
          drupal_set_message($trans->message, 'error');

          // Redirect buyer to the checkout form.
          drupal_goto('checkout/' . $order->order_id . '/review');
        }
      }
      // Stop looping.
      break;
    }
  }
  // Update the order status to pending.
  commerce_order_status_update($order, 'pending');

  // Set a message for the user.
  $message = array(
    'message' => t('Something unexpected happened. Please contact the store administrator.'),
    'type'=> 'error',
  );
  $under_clearance = array(
    'message' => t('Your payment is under a clearance checking by the card operator. A member of our staff will contact you as soon as it gets clearance.'),
    'type'=> 'warning',
  );
  $waiting_capture = array(
    'message' => t('Your payment has been authorized by the card operator but a member of our staff has to manually analyze this transaction. We will contact you as soon as this transaction gets clearance.'),
    'type'=> 'warning',
  );
  $failed = array(
    'message' => t('Your payment was denied by the card operator. Please check with your bank or try a different card.'),
    'type'=> 'error',
  );
  $success = array(
    'message' => t('Your payment has been approved.'),
    'type'=> 'status',
  );

  if (isset($cielo->response['status'])) {
    switch($cielo->response['status']) {
      case CommerceCieloLib::IN_PROGRESS:
      case CommerceCieloLib::AUTHENTICATED:
      case CommerceCieloLib::BEING_AUTHENTICATED:
        // The transaction hasn't been denied but hasn't been
        // approved either, so the store manager must try to capture the money
        // manually.
        $message = $under_clearance;
        break;

      case CommerceCieloLib::AUTHORIZED:
        $message = $waiting_capture;
        break;

      case CommerceCieloLib::AUTHORIZATION_DENIED:
      case CommerceCieloLib::NOT_CAPTURED:
      case CommerceCieloLib::VOIDED:
        $message = $failed;
        break;

      case CommerceCieloLib::CAPTURED:
        $message = $success;
        break;

      case CommerceCieloLib::NOT_AUTHENTICATED:
        if ($authenticate) {
          // Store only accepts transactions that pass by authentication,
          // so we set it as failed.
          $message = $failed;
        }
        else {
          // Although not authenticated, it might still be possible to capture
          // this transaction manually in a later time.
          $message = $under_clearance;
        }
        break;
    }
  }

  drupal_set_message($message['message'], $message['type']);

  // Send the buyer to the checkout complete page.
  drupal_goto('checkout/' . $order->order_id . '/complete');
}
