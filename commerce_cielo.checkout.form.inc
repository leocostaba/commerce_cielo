<?php
/**
 * @file
 * Commerce Cielo checkout pane payment form.
 */

/**
 * Payment method callback: checkout form.
 */
function commerce_cielo_submit_form($payment_method, $pane_values, $checkout_pane, $order) {
  // Set the transaction type ( Credit or Debit ).
  $type = COMMERCE_CIELO_METHOD_CREDIT;
  if (isset($pane_values['payment_details']['credit_card']['type'])) {
    $type = $pane_values['payment_details']['credit_card']['type'];
  }

  // Default flag.
  $flag = COMMERCE_CIELO_CARD_MASTERCARD;

  if (isset($pane_values['payment_details']['credit_card']['flag'])) {
    $flag = $pane_values['payment_details']['credit_card']['flag'];
  }

  $form = array();

  if ($payment_method['settings']['collect_card_details']) {
    // The merchant is in charge of collecting the credit card details.
    module_load_include('inc', 'commerce_payment', 'includes/commerce_payment.credit_card');

    // Get the form fields for collecting the card details.
    $form = commerce_payment_credit_card_form(array('code' => ''));
    // Attach a wrapper div to CVC field.
    $form['credit_card']['code']['#prefix'] = '<div id="credit-card_code_wrapper">';
    $form['credit_card']['code']['#suffix'] = '</div>';

    if ($type == COMMERCE_CIELO_METHOD_DEBIT) {
      // Hides the CVC field.
      $form['credit_card']['code']['#type'] = 'hidden';
      $form['credit_card']['code']['#required'] = FALSE;
    }
  }

  $form['credit_card']['type'] = array(
    '#type' => 'radios',
    '#title' => t('Card Type'),
    '#options' => array(
      COMMERCE_CIELO_METHOD_CREDIT => t('Credit'),
      COMMERCE_CIELO_METHOD_DEBIT => t('Debit')),
    '#weight' => -3,
    '#ajax' => array(
      'callback' => 'commerce_cielo_credit_card_type_ajax_callback',
    ),
    '#default_value' => $type,
  );

  $form['credit_card']['flag'] = array(
    '#type' => 'select',
    '#title' => t('Card Flag'),
    '#options' => _commerce_cielo_flags_by_type($type, $payment_method['settings']),
    '#weight' => -2,
    '#ajax' => array(
      'callback' => 'commerce_cielo_credit_card_flag_ajax_callback',
    ),
    '#prefix' => '<div id="credit-card_flag_wrapper">',
    '#suffix' => '</div>',
    '#default_value' => $flag,
  );

  // Defaults it to hidden.
  $form['credit_card']['installments'] = array(
    '#type' => 'hidden',
    '#title' => t('Number of installments'),
    '#weight' => 0,
    '#default_value' => 1,
    '#prefix' => "<div id='credit-card_installments_wrapper'>",
    '#suffix' => '</div>',
  );

  $currency_code = $order->commerce_order_total['und'][0]['currency_code'];

  if ($currency_code == 'BRL') {
    if ($type == COMMERCE_CIELO_METHOD_CREDIT) {
      // Get settings to build the options list.
      $max_installments = $payment_method['settings']['installments'][$flag . '_max'];
      $po_total = $order->commerce_order_total['und'][0]['amount'];

      // Show installments field.
      $form['credit_card']['installments']['#type'] = 'select';
      $form['credit_card']['installments']['#options'] = _commerce_cielo_installment_options($po_total, $max_installments);
    }
  }

  return $form;
}

/**
 * AJAX callback for CVC and Installment fields.
 */
function commerce_cielo_credit_card_type_ajax_callback($form, $form_state) {

  // Set $settings.
  $settings = $form_state['values']['commerce_payment']['payment_methods']['commerce_cielo|commerce_payment_commerce_cielo']['settings'];

  $currency = $form_state['order']->commerce_order_total['und'][0]['currency_code'];
  $type = $form_state['values']['commerce_payment']['payment_details']['credit_card']['type'];

  // We need to send empty values to the Cielo PHP Library, so that is
  // the why these fields are being hidden instead of removed.
  $form['commerce_payment']['payment_details']['credit_card']['code']['#type'] = 'hidden';
  $form['commerce_payment']['payment_details']['credit_card']['code']['#required'] = FALSE;
  $form['commerce_payment']['payment_details']['credit_card']['installments']['#type'] = 'hidden';

  if ($type === COMMERCE_CIELO_METHOD_CREDIT) {
    // Show CVC field.
    $form['commerce_payment']['payment_details']['credit_card']['code']['#type'] = 'textfield';
    $form['commerce_payment']['payment_details']['credit_card']['code']['#required'] = TRUE;

    if ($currency == 'BRL') {
      $form['commerce_payment']['payment_details']['credit_card']['installments']['#type'] = 'select';
    }
  }

  // Replace the form element options.
  $form['commerce_payment']['payment_details']['credit_card']['flag']['#options'] = _commerce_cielo_flags_by_type($type, $settings);

  return array(
    '#type' => 'ajax',
    '#commands' => array(
      ajax_command_replace("#credit-card_code_wrapper", render($form['commerce_payment']['payment_details']['credit_card']['code'])),
      ajax_command_replace("#credit-card_flag_wrapper", render($form['commerce_payment']['payment_details']['credit_card']['flag'])),
      ajax_command_replace("#credit-card_installments_wrapper", render($form['commerce_payment']['payment_details']['credit_card']['installments'])),
    ),
  );
}

/**
 * AJAX callback - Calculates the max number of installments.
 */
function commerce_cielo_credit_card_flag_ajax_callback($form, $form_state) {
  $currency = $form_state['order']->commerce_order_total['und'][0]['currency_code'];
  $type = $form_state['values']['commerce_payment']['payment_details']['credit_card']['type'];

  if ($currency == 'BRL' && $type == COMMERCE_CIELO_METHOD_CREDIT) {
    // Get the current flag installment settings.
    $flag = $form_state['values']['commerce_payment']['payment_details']['credit_card']['flag'];
    $max_installments = $form_state['order']->payment_methods['commerce_cielo|commerce_payment_commerce_cielo']['settings']['installments'][$flag . '_max'];
    $po_total = $form_state['order']->commerce_order_total['und'][0]['amount'];

    // Rebuilt the list of options.
    $form['commerce_payment']['payment_details']['credit_card']['installments']['#options'] = _commerce_cielo_installment_options($po_total, $max_installments);
  }

  return array(
    '#type' => 'ajax',
    '#commands' => array(
      ajax_command_replace("#credit-card_installments_wrapper", render($form['commerce_payment']['payment_details']['credit_card']['installments'])),
    ),
  );
}

/**
 * Payment method callback: checkout form validation.
 */
function commerce_cielo_submit_form_validate($payment_method, $pane_form, $pane_values, $order, $form_parents = array()) {
  if ($payment_method['settings']['collect_card_details']) {
    module_load_include('inc', 'commerce_payment', 'includes/commerce_payment.credit_card');

    // Validate the credit card fields.
    $settings = array(
      'form_parents' => array_merge($form_parents, array('credit_card')),
    );

    if (!commerce_payment_credit_card_validate($pane_values['credit_card'], $settings)) {
      return FALSE;
    }
  }
}

/**
 * Payment method callback: checkout form submission.
 */
function commerce_cielo_submit_form_submit($payment_method, $pane_form, $pane_values, $order, $charge) {
  $response = commerce_cielo_authorize($payment_method['settings'], $pane_values, $order, $charge);


  if (empty($response)) {
    return FALSE;
  }

  // Prepare a transaction object to log the Webservice response.
  $transaction = commerce_payment_transaction_new('commerce_cielo', $order->order_id);
  $transaction->instance_id = $payment_method['instance_id'];
  $transaction->amount = $charge['amount'];
  $transaction->currency_code = $charge['currency_code'];

  $remote_status = _commerce_cielo_get_remote_status($response, $payment_method['settings']['auto_capture']);

  // Save the transaction.
  commerce_cielo_save_transaction($response, $transaction, $payment_method['settings']['authenticate'], $remote_status);

  // Rebuilds the form if authorize transaction fails.
  if ($transaction->status == COMMERCE_PAYMENT_STATUS_FAILURE) {
    drupal_set_message(t('We received the following error processing your card. Please enter you information again or try a different card.'), 'error');
    drupal_set_message($transaction->message, 'error');
    return FALSE;
  }

  if (!empty($response->{'url-autenticacao'})) {
    // Send browser to Cielo for collecting card details and/or performing
    // ownership authentication.
    drupal_goto($response->{'url-autenticacao'});
  }

  // CAPTURING.
  // At this point there was no redirection, so we try to capture.
  // We check if auto_capture is switched on and if there is a pending response
  // from the last call to the webservice.
  if ($payment_method['settings']['auto_capture'] == 'true' &&
      $transaction->status == COMMERCE_CIELO_AUTHORIZED) {

    // Try to capture.
    $response = commerce_cielo_capture($payment_method['settings'], $transaction);

    // Save the transaction.
    commerce_cielo_save_transaction($response, $transaction, $payment_method['settings']['authenticate']);

    // Rebuilds the form if capture transaction fails.
    if ($transaction->status == COMMERCE_PAYMENT_STATUS_FAILURE) {
      drupal_set_message(t('We received the following error processing your card. Please enter you information again or try a different card.'), 'error');
      drupal_set_message($transaction->message, 'error');
      return FALSE;
    }
  }
}

/**
 * Helper function to filter enabled flags by type.
 */
function _commerce_cielo_flags_by_type($type, $settings = array()) {
  if ($type == COMMERCE_CIELO_METHOD_CREDIT) {
    $flag_support_setting = 'credit_mode_flag_support';
  }
  elseif ($type == COMMERCE_CIELO_METHOD_DEBIT) {
    $flag_support_setting = 'debit_mode_flag_support';
  }
  else {
    return array();
  }
  $flag_options = commerce_cielo_card_types();

  // Remove the flags that are not supported for the current selected card type.
  foreach ($flag_options as $key => $value) {
    if (!$settings['tuning'][$flag_support_setting][$key]) {
      unset($flag_options[$key]);
    }
  }

  return $flag_options;
}
