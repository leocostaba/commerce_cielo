<?php
/**
 * @file
 * Concludes the payment process after returning from Cielo.
 */

/**
 * Callback Function for checkout/%commerce_order/payment/cielo.
 *
 * This page and function are trigged only when there is a redirection back from Cielo.
 *
 */
function commerce_cielo_redirect_back($order) {
  global $user;
  // Make sure the user is logged in.
  if (!in_array('authenticated user', $user->roles)) {
    drupal_set_message(t('Please, login to conclude your checkout payment process.'), 'warning');

    // Send the user to the login form page and fill in the url query value of destination with the current url path
    // so he/she is redirected back here again.
    $destination = drupal_get_destination();
    drupal_goto('user', array('query' => array('destination' => $destination['destination'])));
  }
  // Making sure the PO belongs to the current logged in user.
  if ($user->uid == $order->uid) {
      // Load payment transactions that belongs to this order.
      foreach (commerce_payment_transaction_load_multiple(array(), array('order_id' => $order->order_id)) as $transaction) {
        if ($transaction->remote_status == 'redirected' && $order->status == 'checkout_review') {
          // Load the payment method settings.
          $payment_method = commerce_payment_method_instance_load($transaction->instance_id);

          // Prepare the arguments to pass to object instantiantion of the external library class.
          $arguments = commerce_cielo_get_arguments($payment_method, $order->order_number, $transaction->amount);

          // Instantiate a object from the external library.
          $cielo = new Cielo($arguments);

          // Set the tid assigned by Cielo.
          $cielo->setTid($transaction->remote_id);

          // Only for testing enviroment
          if ($payment_method['settings']['is_test']) {
            // Make sure we use the same credentials previously used in the checkout.
            $cielo->membership['filiacao'] = $_COOKIE['filiacao'];
            $cielo->membership['chave'] = $_COOKIE['chave'];
          }

          // Look up on the transaction created while away at Cielo.
          $cielo->followUp();

          // Check for and save any warnings that might have been issued by the external library (Brazilcards).
          commerce_cielo_logWatchDog($cielo);

          $message = '';
          if (isset($cielo->response['status'])) {
            // No transaction errors have occured.
            $status = check_plain($cielo->response['status']);
            $message = t('Status Code: %code (Follow Up)', array('%code' => $status));

            foreach ($cielo->response as $process => $values) {
              if (is_array($values)) {
                // Payment tab message.
                if (key_exists('mensagem', $values)) {
                  // Assemble payment tab message
                  $message .= '<br />' . check_plain(t("$process:")) . ' ' . check_plain($values['mensagem']);
                }

              }
            }
          }

          $authenticate = !isset($payment_method['settings']['authenticate'])?TRUE:$payment_method['settings']['authenticate'];

          if (isset($cielo->response['status']) && $cielo->response['status'] == COMMERCE_CIELO_AUTHORIZED
              && !$payment_method['settings']['auto_capture']) {
            // Authorization has been granted and capturing will be done later at the store backend
            $remote_status = 'authorization_only';
          }
          else{
            $remote_status = commerce_cielo_get_status_state($authenticate, $status);
          }

          // Prepare a transaction object to log the Webservice response.
          $trans = commerce_payment_transaction_new('cielo', $order->order_id);
          $trans->instance_id = $payment_method['instance_id'];
          $trans->amount = $transaction->amount;
          $trans->currency_code = $transaction->currency_code;
          $trans->remote_id = $transaction->remote_id;
          $trans->remote_status = $remote_status;
          $trans->status = commerce_cielo_get_status_state($authenticate, $status);
          $trans->message = $message;
          $trans->payload[REQUEST_TIME] = $cielo->response;

          // Save the follow up transaction.
          commerce_payment_transaction_save($trans);

           // Rebuilds the checkout review form if follow up transaction fails.
          if ($trans->status == COMMERCE_PAYMENT_STATUS_FAILURE) {
            drupal_set_message(t('We received the following error processing your card. Please enter you information again or try a different card.'), 'error');
            drupal_set_message($trans->message, 'error');

            // Redirect buyer to the checkout form
            drupal_goto('checkout/' . $order->order_id . '/review');
          }

          // Capturing

          if (isset($cielo->response['status']) && $cielo->response['status'] == COMMERCE_CIELO_AUTHORIZED
              && $payment_method['settings']['auto_capture']) {
            // Authorization has been granted and auto capture is switched on, so we will try to capture.
            $cielo->capture();

            // Check for and save any warnings that might have been issued by the external library (Brazilcards).
            commerce_cielo_logWatchDog($cielo);

            // Save the capturing transaction.
            commerce_cielo_saveTransaction($cielo, $trans, $authenticate);

            // Rebuilds the checkout review form if follow up transaction fails.
            if ($trans->status == COMMERCE_PAYMENT_STATUS_FAILURE) {
              drupal_set_message(t('We received the following error processing your card. Please enter you information again or try a different card.'), 'error');
              drupal_set_message($trans->message, 'error');

              // Redirect buyer to the checkout form
              drupal_goto('checkout/' . $order->order_id . '/review');
            }
          }
          // Stop looping
          break;
        }
      }
      // Now the payment transaction should have either a success or pending status, so we update the order status to pending
      commerce_order_status_update($order, 'pending');

      // Send the buyer to the checkout complete page
      drupal_goto('checkout/' . $order->order_id . '/complete');
  }
  else{
    // Interropts the flow and sends the browser to the home page
    drupal_set_message(t('Sorry, but apparently the order #%po does not belong to you. Please contact the store administrator.', array('%po' => $order->order_number)), 'error');
    drupal_goto();
  }
}
