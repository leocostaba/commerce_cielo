<?php
/**
 * @file
 * Administrative forms for the Cielo module.
 */

/**
 * Form callback: allows the user to capture a prior authorization.
 */
function commerce_cielo_capture_form($form, &$form_state, $order, $transaction) {
  $form_state['order']       = $order;
  $form_state['transaction'] = $transaction;

  // Load and store the payment method instance for this transaction.
  $payment_method = commerce_payment_method_instance_load($transaction->instance_id);
  $form_state['payment_method'] = $payment_method;

  $balance = commerce_payment_order_balance($order);

  if ($balance['amount'] > 0 && $balance['amount'] < $transaction->amount) {
    $default_amount = $balance['amount'];
  }
  else {
    $default_amount = $transaction->amount;
  }

  // Convert the price amount to a user friendly decimal value.
  $default_amount = commerce_currency_amount_to_decimal($default_amount, $transaction->currency_code);

  $form['amount'] = array(
    '#type' => 'textfield',
    '#title' => t('Capture amount'),
    '#description' => implode('<br />', array(
      t('Authorization: @amount', array('@amount' => commerce_currency_format($transaction->amount, $transaction->currency_code))),
      t('Order balance: @balance', array('@balance' => commerce_currency_format($balance['amount'], $balance['currency_code']))),
    )),
    '#default_value' => $default_amount,
    '#field_suffix' => check_plain($transaction->currency_code),
    '#size' => 16,
  );

  $form = confirm_form($form,
    t('What amount do you want to capture?'),
    'admin/commerce/orders/' . $order->order_id . '/payment',
    '',
    t('Capture'),
    t('Cancel'),
    'confirm'
  );

  return $form;
}

/**
 * Validate handler: ensure a valid amount is given.
 */
function commerce_cielo_capture_form_validate($form, &$form_state) {
  $transaction    = $form_state['transaction'];
  $amount         = $form_state['values']['amount'];

  // Ensure a positive numeric amount has been entered for capture.
  if (!is_numeric($amount) || $amount <= 0) {
    form_set_error('amount', t('You must specify a positive numeric amount to capture.'));
  }

  // Ensure the amount is less than or equal to the authorization amount.
  if ($amount > commerce_currency_amount_to_decimal($transaction->amount, $transaction->currency_code)) {
    form_set_error('amount', t('You cannot capture more than it has been authorized at the checkout.'));
  }

  // If the authorization has expired, display an error message and redirect.
  if (REQUEST_TIME - $transaction->created > 86400 * 5) {
    drupal_set_message(t('This authorization has passed its 5 days limit and cannot be captured.'), 'error');
    drupal_goto('admin/commerce/orders/' . $form_state['order']->order_id . '/payment');
  }
}

/**
 * Submit handler: process a prior authorization capture.
 */
function commerce_cielo_capture_form_submit($form, &$form_state) {

  $transaction = $form_state['transaction'];
  $amount      = $form_state['values']['amount'];

  // Prepare the arguments to pass to the object instantiantion of the external
  // library class.
  $arguments = _commerce_cielo_get_arguments($form_state['payment_method'], $form_state['order']->order_number, $transaction->amount);

  // Instantiate a object from the external library.
  $cielo = new CommerceCieloCielo($arguments);

  // Set the tid assigned by Cielo.
  $cielo->setTid($transaction->remote_id);

  $cielo->capturePreAuthorize($amount);

  // Check for and save any warnings that might have been issued by the external
  // library (Brazilcards).
  commerce_cielo_log_watch_dog($cielo);

  // Update and save the transaction based on the response.
  $transaction->payload[REQUEST_TIME] = $cielo->response;

  $message = _commerce_cielo_set_status_message($cielo->response);

  if (isset($cielo->response['status']) && $cielo->response['status'] == CommerceCieloCielo::CIELO_CAPTURED) {
    drupal_set_message(t('Transaction has been captured successfully.'));

    // Update the transaction amount to the actual capture amount.
    $transaction->amount = commerce_currency_decimal_to_amount($amount, $transaction->currency_code);

    // Set the remote and local status accordingly.
    $transaction->status        = COMMERCE_PAYMENT_STATUS_SUCCESS;
    $transaction->remote_status = COMMERCE_PAYMENT_STATUS_SUCCESS;

  }
  else {
    // Display an error message but leave the transaction pending.
    drupal_set_message(t('Capturing failed, so the transaction will remain in a pending status.'), 'error');
    drupal_set_message($message['message'], 'error');
  }

  $transaction->message = $message['message'];
  commerce_payment_transaction_save($transaction);

  $form_state['redirect'] = 'admin/commerce/orders/' . $form_state['order']->order_id . '/payment';
}


/**
 * Form callback: allows the user to void a captured transaction.
 */
function commerce_cielo_void_form($form, &$form_state, $order, $transaction) {

  $form_state['order']       = $order;
  $form_state['transaction'] = $transaction;

  // Load and store the payment method instance for this transaction.
  $payment_method = commerce_payment_method_instance_load($transaction->instance_id);
  $form_state['payment_method'] = $payment_method;

  $form = confirm_form($form,
    t('Are you sure you wanna void this transaction and refund the buyer?'),
    'admin/commerce/orders/' . $order->order_id . '/payment',
    '',
    t('Void'),
    t('Cancel'),
    'confirm'
  );

  return $form;
}

/**
 * Submit handler: process a prior authorization capture.
 */
function commerce_cielo_void_form_submit($form, &$form_state) {
  $transaction = $form_state['transaction'];

  // Prepare the arguments to pass to object instantiantion of the external
  // library class.
  $arguments = _commerce_cielo_get_arguments($form_state['payment_method'], $form_state['order']->order_number, $transaction->amount);

  // Instantiate a object from the external library.
  $cielo = new CommerceCieloCielo($arguments);

  // Set the tid assigned by Cielo.
  $cielo->setTid($transaction->remote_id);

  $cielo->voidTransaction();

  // Check for and save any warnings that might have been issued by the external
  // library (Brazilcards).
  commerce_cielo_log_watch_dog($cielo);

  // Update and save the transaction based on the response.
  $transaction->payload[REQUEST_TIME] = $cielo->response;

  $message = _commerce_cielo_set_status_message($cielo->response);

  $form_state['redirect'] = 'admin/commerce/orders/' . $form_state['order']->order_id . '/payment';
}

/**
 * Helper function.
 * Generates a user friendly status / error message fetched
 * from the remote server.
 *
 * @param array $reponse
 *   The webservice response array.
 */
function _commerce_cielo_set_status_message($reponse) {
  $message = array(
    'status'  => '',
    'message' => '',
  );

  if (isset($reponse['status'])) {
    // No transaction errors have occured.
    $message['status']  = $reponse['status'];
    $message['message'] = t('Status Code: @code', array('@code' => $message['status']));

    foreach ($reponse as $process => $values) {
      if (is_array($values)) {
        // Payment tab message.
        if (key_exists('mensagem', $values)) {
          // Assemble payment tab message.
          $message['message'] .= '<br />' . t("@process: @message", array('@process' => $process, '@message' => $values['mensagem']));
        }
      }
    }
  }
  elseif (isset($reponse['codigo'])) {
    // The remote server reported an error.
    $message['status'] = check_plain($reponse['codigo']);
    $message['message'] = t('Error Code: @code', array('@code' => $reponse['codigo']));
    $message['message'] .= '<br />' . t('Error Message: @message', array('@message' => $reponse['mensagem']));
  }

  return $message;
}
