<?php

/**
 * Payment method callback: checkout form.
 */
function commerce_cielo_submit_form($payment_method, $pane_values, $checkout_pane, $order) {
  // dprint_r($payment_method['settings']['tuning']);

  // Check if the external library (Brazilcards) is installed.
  if (!libraries_get_path('brazilcards-lib')) {
    // Log it to the system log.
    watchdog('commerce_cielo', 'The BrazilCards Library could not be found or is not properly installed.', array(), WATCHDOG_DEBUG);
    // Notify the user.
    drupal_set_message(t('Sorry for this but the checkout system is misconfigured, please report this to the website administrator.'), 'error');
    // Send the user to the home page.
    drupal_goto();
  }

  global $user;

  // Make sure the user is not anonymous.
  if (!in_array('authenticated user', $user->roles)) {

    drupal_set_message(t('Please, login to conclude your checkout payment process.'), 'warning');
    // Get the current url for redirecting the user back to the checkout review
    // page after he/she logs in.
    $destination = drupal_get_destination();
    // Send the user to the login page.
    drupal_goto('user', array('query' => array('destination' => $destination['destination'])));
  }

  // Possible values for type are 1 == Credit Card or A == Debit Card.
  $type = Cielo::CIELO_TYPE_CREDIT_CARD;
  if (isset($pane_values['payment_details']['credit_card']['type'])) {
    $type = $pane_values['payment_details']['credit_card']['type'];
  }

  // Default flag.
  $flag = Cielo::CIELO_FLAG_MASTERCARD;

  if (isset($pane_values['payment_details']['credit_card']['flag'])) {
    $flag = $pane_values['payment_details']['credit_card']['flag'];
  }

  if ($payment_method['settings']['collect_card_details']) {
    // The merchant is in charge of collecting the credit card details.
    module_load_include('inc', 'commerce_payment', 'includes/commerce_payment.credit_card');

    if ($type == Cielo::CIELO_TYPE_CREDIT_CARD) {
      // CVC field shows up on the form.
      $form = commerce_payment_credit_card_form(array('code' => ''));
      // Attach a wrapper div to cvc field.
      $form['credit_card']['code']['#prefix'] = '<div id="credit-card_code_wrapper">';
      $form['credit_card']['code']['#suffix'] = '</div>';
    }
    else {
      // CVC doesn't show up when Debit Card option is/was selected.
      $form = commerce_payment_credit_card_form();
    }
  }

  $form['credit_card']['type'] = array(
    '#type' => 'radios',
    '#title' => t('Card Type'),
    '#options' => array(
      Cielo::CIELO_TYPE_CREDIT_CARD => t('Credit'),
      Cielo::CIELO_TYPE_DEBIT_CARD => t('Debit')),
    '#weight' => -3,
    '#ajax' => array(
      'callback' => 'commerce_cielo_credit_card_type_ajax_callback',
    ),
    '#default_value' => $type,
  );

  $form['credit_card']['flag'] = array(
    '#type' => 'select',
    '#title' => t('Card Flag'),
    '#options' => Cielo::get_card_flags(TRUE, 'commerce_cielo_t'),
    '#weight' => -2,
    '#ajax' => array(
      'callback' => 'commerce_cielo_credit_card_flag_ajax_callback',
    ),
    '#prefix' => '<div id="credit-card_flag_wrapper">',
    '#suffix' => '</div>',
    '#default_value' => $flag,
  );

  // Make it hidden and set value to 1.
  $form['credit_card']['installments'] = array(
    '#type' => 'hidden',
    '#title' => t('Number of installments'),
    '#weight' => 0,
    '#default_value' => 1,
    '#prefix' => '<div id="credit-card_installments_wrapper">',
    '#suffix' => '</div>',
  );

  $currency_code = $order->commerce_order_total['und'][0]['currency_code'];

  if ($currency_code == 'BRL') {
    // Attach ajax callback function to allow updating of installment field.
    $form['credit_card']['type'] += array(
      '#ajax' => array(
        'callback' => 'commerce_cielo_credit_card_type_ajax_callback',
      ),
    );
    if ($type == 1) {
      // Add the installment field.
      // Attach ajax attributes to flag so it can update the max number of
      // installments on the installment field.
      $form['credit_card']['flag'] += array(
        '#ajax' => array(
          'callback' => 'commerce_cielo_credit_card_flag_ajax_callback',
          'wrapper' => 'credit-card_installments_wrapper',
        ),
      );

      $max_installments = $payment_method['settings']['installments'][$flag . '_max'];

      $po_total = $order->commerce_order_total['und'][0]['amount'];

      $form['credit_card']['installments'] = array(
        '#type' => 'select',
        '#title' => t('Number of installments'),
        '#options' => commerce_cielo_installment_options($po_total, $max_installments),
        '#weight' => -1,
        '#default_value' => 1,
        '#prefix' => '<div id="credit-card_installments_wrapper">',
        '#suffix' => '</div>',
      );
    }
  }

  if ($payment_method['settings']['collect_card_details']) {

    $form['credit_card']['code'] = array(
      '#type' => 'textfield',
      '#title' => t('Security Code'),
      '#description' => t('Enter the 3 digits number located at the back of your card.'),
      '#size' => 3,
      '#prefix' => '<div id="credit-card_code_wrapper">',
      '#suffix' => '</div>',
    );

    if ($type == Cielo::CIELO_TYPE_DEBIT_CARD) {
      $form['credit_card']['code']['#type'] = 'hidden';
      $form['credit_card']['code']['#required'] = FALSE;
    }
  }

  return $form;
}

/**
 * AJAX callback for CVC and Installment fields.
 */
function commerce_cielo_credit_card_type_ajax_callback($form, $form_state) {
  // Set $settings. 
  $settings = $form_state['values']['commerce_payment']['payment_methods']['cielo|commerce_payment_cielo']['settings'];

  $currency = $form_state['order']->commerce_order_total['und'][0]['currency_code'];
  $type = $form_state['values']['commerce_payment']['payment_details']['credit_card']['type'];

  // We need to send empty values to the BrazilCards library, so that is the why
  // these fields are being hidden instead of removed.
  $form['credit_card']['code'] = array(
    '#type' => 'hidden',
    '#prefix' => '<div id="credit-card_code_wrapper">',
    '#suffix' => '</div>',
  );

  $form['credit_card']['installments'] = array(
    '#type' => 'hidden',
    '#default_value' => 1,
    '#prefix' => '<div id="credit-card_installments_wrapper">',
    '#suffix' => '</div>',
  );
  
  // Set default.
  $flag_support_setting = 'debit_mode_flag_support';
 
  if ($type === Cielo::CIELO_TYPE_CREDIT_CARD) {
  
    $flag_support_setting = 'credit_mode_flag_support';
    
    // Reconstruct cvc field.
    $form['credit_card']['code'] = array(
      '#type' => 'textfield',
      '#title' => t('Security Code'),
      '#description' => t('Enter the 3 digits number located at the back of your card.'),
      '#size' => 3,
      '#prefix' => '<div id="credit-card_code_wrapper">',
      '#suffix' => '</div>',
    );

    if ($currency == 'BRL') {
      // Brazilian Real.
      $form['credit_card']['installments'] = array(
        '#type' => 'select',
        '#title' => t('Number of installments'),
        '#options' => $form['commerce_payment']['payment_details']['credit_card']['installments']['#options'],
        '#weight' => 0,
        '#default_value' => 1,
        '#prefix' => '<div id="credit-card_installments_wrapper">',
        '#suffix' => '</div>',
      );
    }
  }
 
  $flag_options = Cielo::get_card_flags(TRUE, 'commerce_cielo_t');
  
  // Remove the flags that are not supported for the selected card type (either
  // Credit or Debit).
  foreach ($flag_options as $key => $value) {
    if (!$settings['tuning'][$flag_support_setting][$key]) {
      unset($flag_options[$key]);
    }
  }

  $form['credit_card']['flag'] = array(
    '#type' => 'select',
    '#title' => t('Card Flag'),
    '#options' => $flag_options,
    '#weight' => -2,
    '#ajax' => array(
      'callback' => 'commerce_cielo_credit_card_flag_ajax_callback',
    ),
    '#prefix' => '<div id="credit-card_flag_wrapper">',
    '#suffix' => '</div>',
  );

  return array(
    '#type' => 'ajax',
    '#commands' => array(
      ajax_command_replace("#credit-card_flag_wrapper", render($form['credit_card']['flag'])),
      ajax_command_replace("#credit-card_code_wrapper", render($form['credit_card']['code'])),
      ajax_command_replace("#credit-card_installments_wrapper", render($form['credit_card']['installments'])),
    ),
  );
}

/**
 * AJAX callback for calculating the max number of installments for the
 * installment field.
 */
function commerce_cielo_credit_card_flag_ajax_callback($form, $form_state) {

  $currency = $form_state['order']->commerce_order_total['und'][0]['currency_code'];
  $type = $form_state['values']['commerce_payment']['payment_details']['credit_card']['type'];

  if ($currency == 'BRL' && $type == Cielo::CIELO_TYPE_CREDIT_CARD) {

    $flag = $form_state['values']['commerce_payment']['payment_details']['credit_card']['flag'];
    $max_installments = $form_state['order']->payment_methods['cielo|commerce_payment_cielo']['settings']['installments'][$flag . '_max'];
    $po_total = $form_state['order']->commerce_order_total['und'][0]['amount'];

    $form['credit_card']['installments'] = array(
      '#type' => 'select',
      '#title' => t('Number of installments'),
      '#options' => commerce_cielo_installment_options($po_total, $max_installments),
      '#weight' => -1,
      '#default_value' => 1,
      '#prefix' => '<div id="credit-card_installments_wrapper">',
      '#suffix' => '</div>',
    );

    return $form['credit_card']['installments'];
  }
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

    // Check if this is a debit card.
    if ($pane_values['credit_card']['type'] == Cielo::CIELO_TYPE_DEBIT_CARD) {
      // Remove cvc field from being validated.
      unset($pane_values['credit_card']['code']);
    }

    if (!commerce_payment_credit_card_validate($pane_values['credit_card'], $settings)) {
      return FALSE;
    }
  }
}

/**
 * Payment method callback: checkout form submission.
 */
function commerce_cielo_submit_form_submit($payment_method, $pane_form, $pane_values, $order, $charge) {

  // Define whether or not the card holder ownership should be authenticated.
  $authenticate = !isset($payment_method['settings']['authenticate']) ? TRUE : $payment_method['settings']['authenticate'];

  if ($pane_values['credit_card']['type'] == Cielo::CIELO_TYPE_DEBIT_CARD) {
    $payment_method['settings']['authorization_type'] = Cielo::CIELO_AUTHORIZE_ONLY_IF_AUTHENTICATED;
  }

  // Prepare the arguments to pass to the object instantiantion of the external
  // library class.
  $arguments = commerce_cielo_get_arguments($payment_method, $order->order_number, $charge['amount']);

  // Attach the payment details for the authorize request.
  $arguments['payment'] = array(
    'CardFlag' => $pane_values['credit_card']['flag'],
    'Installments' => $pane_values['credit_card']['installments'],
    'Creditor' => $payment_method['settings']['installments']['creditor'],
    'CardType' => $pane_values['credit_card']['type'],
    'Authenticate' => $authenticate,
    'AuthorizationType' => $payment_method['settings']['authorization_type'],
    'AutoCapturer' => $payment_method['settings']['auto_capture'],
  );

  if ($payment_method['settings']['collect_card_details']) {
    // Attach the card details to the argument's array.
    $arguments['payment'] += array(
      'CardNumber' => $pane_values['credit_card']['number'],
      'ExpirationYear' => $pane_values['credit_card']['exp_year'],
      'ExpirationMonth' => $pane_values['credit_card']['exp_month'],
      'CVC' => $pane_values['credit_card']['code'],
    );
  }

  // Get the ISO 4217 info about all the enabled currencies.
  $currencies = commerce_currencies(TRUE);

  // Get the currency code formatted as 3 digits number. ie. 840 for USD.
  foreach ($currencies as $tree_letter_code => $value) {
    if ($tree_letter_code == $charge['currency_code']) {
      $currency_numeric_code = $value['numeric_code'];
      break;
    }
  }

  // Instantiate the external library.
  $Cielo = new Cielo($arguments);

  if ($payment_method['settings']['is_test']) {
    setcookie('filiacao', $Cielo->membership['filiacao']);
    setcookie('chave', $Cielo->membership['chave']);
  }

  // Set the currency code.
  $Cielo->setCurrency($currency_numeric_code);

  // Set the language interface.
  $Cielo->setLanguage($payment_method['settings']['language']);

  // Set the url for returning from Cielo.
  $url = url('checkout/' . $order->order_number . '/payment/cielo', array('absolute' => TRUE));
  $Cielo->setReturnUrl($url);

  // Request Authorization.
  $Cielo->authorize();

  // Check for and save any warnings that might have been issued by the external
  // library (Brazilcards).
  commerce_cielo_log_watch_dog($Cielo);

  // Prepare a transaction object to log the Webservice response.
  $transaction = commerce_payment_transaction_new('cielo', $order->order_id);
  $transaction->instance_id = $payment_method['instance_id'];
  $transaction->amount = $charge['amount'];
  $transaction->currency_code = $charge['currency_code'];

  $remote_status = '';
  if (isset($Cielo->response['status']) && $Cielo->response['status'] == Cielo::CIELO_AUTHORIZED
      && !$payment_method['settings']['auto_capture']) {
    // Authorization has been granted and capturing will be done later at the
    // store backend.
    $remote_status = 'authorization_only';
  }
  // Save the transaction.
  commerce_cielo_save_transaction($Cielo, $transaction, $authenticate, $remote_status);

   // Rebuilds the form if autorize transaction fails.
  if ($transaction->status == COMMERCE_PAYMENT_STATUS_FAILURE) {
    drupal_set_message(t('We received the following error processing your card. Please enter you information again or try a different card.'), 'error');
    drupal_set_message($transaction->message, 'error');
    return FALSE;
  }

  if (isset($Cielo->response['url-autenticacao'])) {
    // Send browser to Cielo for collecting card details and/or performing
    // ownership authentication.
    drupal_goto($Cielo->response['url-autenticacao']);
  }

  // CAPTURING.
  // At this point there was no redirection, so we try to capture.
  // We check if auto_capture is switched on and if there is a pending response
  // from the last call to the webservice.
  if ($payment_method['settings']['auto_capture'] && $transaction->status == Cielo::CIELO_AUTHORIZED) {
    // Try to capture.
    $Cielo->capture();

    // Check for and save any warnings that might have been issued by the
    // external library (Brazilcards).
    commerce_cielo_log_watch_dog($Cielo);

    // Save the transaction.
    commerce_cielo_save_transaction($Cielo, $transaction, $authenticate);

    // Rebuilds the form if capture transaction fails.
    if ($transaction->status == COMMERCE_PAYMENT_STATUS_FAILURE) {
      drupal_set_message(t('We received the following error processing your card. Please enter you information again or try a different card.'), 'error');
      drupal_set_message($transaction->message, 'error');
      return FALSE;
    }
  }
}
