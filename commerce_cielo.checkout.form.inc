<?php
/**
 * @file
 * Commerce Cielo checkout pane payment form.
 */

/**
 * Payment method callback: checkout form.
 */
function commerce_cielo_submit_form($payment_method, $pane_values, $checkout_pane, $order) {
  // Check if the external library (Brazilcards) is installed.
  if (!libraries_get_path('brazilcards-lib')) {
    // Log it to the system log.
    watchdog('commerce_cielo', 'The BrazilCards Library could not be found or is not properly installed.', array(), WATCHDOG_DEBUG);
    // Notify the user.
    drupal_set_message(t('Sorry for this but the checkout system is misconfigured, please report this to the website administrator.'), 'error');
    // Send the user to the home page.
    drupal_goto('checkout');
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

  $form = array();

  if ($payment_method['settings']['collect_card_details']) {
    // The merchant is in charge of collecting the credit card details.
    module_load_include('inc', 'commerce_payment', 'includes/commerce_payment.credit_card');

    if ($type == Cielo::CIELO_TYPE_CREDIT_CARD) {
      // CVC field shows up on the form.
      $form = commerce_payment_credit_card_form(array('code' => ''));
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
    '#default_value' => $flag,
  );

  // Make it hidden and set value to 1.
  $form['credit_card']['installments'] = array(
    '#type' => 'hidden',
    '#title' => t('Number of installments'),
    '#weight' => 0,
    '#default_value' => 1,
  );

  $currency_code = $order->commerce_order_total['und'][0]['currency_code'];

  if ($currency_code == 'BRL') {
    if ($type == Cielo::CIELO_TYPE_CREDIT_CARD) {
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
        '#options' => _commerce_cielo_installment_options($po_total, $max_installments),
        '#weight' => -1,
        '#default_value' => 1,
      );
    }
  }

  if ($payment_method['settings']['collect_card_details']) {

    $form['credit_card']['code'] = array(
      '#type' => 'textfield',
      '#title' => t('Security Code'),
      '#description' => t('Enter the 3 digits number located at the back of your card.'),
      '#size' => 3,
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
  $card_cvs_code = '<div class="form-item form-type-textfield form-item-commerce-payment-payment-details-credit-card-code" style="display:none">
                      <label for="edit-commerce-payment-payment-details-credit-card-code">' . t('Security Code') . '</label>
                        <input type="text" id="edit-commerce-payment-payment-details-credit-card-code" name="commerce_payment[payment_details][credit_card][code]" value="" size="3" maxlength="128" class="form-text" />
                        <div class="description">' . t('Enter the 3 digits number located at the back of your card.') . '</div>
                    </div>';

  $installments_field = '<div class="form-item form-type-select form-item-commerce-payment-payment-details-credit-card-installments" style="display:none">
                          <label for="edit-commerce-payment-payment-details-credit-card-installments">' . t('Number of installments') . '</label>
                          <select id="edit-commerce-payment-payment-details-credit-card-installments" name="commerce_payment[payment_details][credit_card][installments]" class="form-select">
                            <option value="1" selected="selected">' . t('Single pay out') . '</option>
                          </select>
                        </div>';

  // Set default.
  $flag_support_setting = 'debit_mode_flag_support';

  if ($type === Cielo::CIELO_TYPE_CREDIT_CARD) {

    $flag_support_setting = 'credit_mode_flag_support';

    // Reconstruct cvc field.
    $card_cvs_code = '<div class="form-item form-type-textfield form-item-commerce-payment-payment-details-credit-card-code">
                        <label for="edit-commerce-payment-payment-details-credit-card-code">' . t('Security Code') . '</label>
                          <input type="text" id="edit-commerce-payment-payment-details-credit-card-code" name="commerce_payment[payment_details][credit_card][code]" value="" size="3" maxlength="128" class="form-text" />
                          <div class="description">' . t('Enter the 3 digits number located at the back of your card.') . '</div>
                      </div>';

    if ($currency == 'BRL') {

      $installments_field_array = $form['commerce_payment']['payment_details']['credit_card']['installments']['#options'];
      $installments_field_options = '';

      foreach ($installments_field_array as $key => $value) {
        $installments_field_options .= "<option value='$key'>$value</option>";
      }

      $installments_field = '<div class="form-item form-type-select form-item-commerce-payment-payment-details-credit-card-installments">
                              <label for="edit-commerce-payment-payment-details-credit-card-installments">' . t('Number of installments') . '</label>
                              <select id="edit-commerce-payment-payment-details-credit-card-installments" name="commerce_payment[payment_details][credit_card][installments]" class="form-select">' .
                                $installments_field_options .
                              '</select>
                            </div>';
    }
  }

  $flag_options_array = Cielo::get_card_flags(TRUE, 'commerce_cielo_t');
  $flag_options = '';
  // Construct the html tag options for Card Flag field.
  foreach ($flag_options_array as $key => $value) {
    if ($settings['tuning'][$flag_support_setting][$key]) {
      $flag_options .= "<option value='$key'>$value</option>";
    }
  }

  $card_flag_options = '<select id="edit-commerce-payment-payment-details-credit-card-flag" ' .
                          'name="commerce_payment[payment_details][credit_card][flag]"' .
                          'class="form-select">' .
                          $flag_options .
                        '</select>';

  return array(
    '#type' => 'ajax',
    '#commands' => array(
      ajax_command_replace("#edit-commerce-payment-payment-details-credit-card-flag", $card_flag_options),
      ajax_command_replace(".form-item-commerce-payment-payment-details-credit-card-code", $card_cvs_code),
      ajax_command_replace(".form-item-commerce-payment-payment-details-credit-card-installments", $installments_field),
    ),
  );
}

/**
 * AJAX callback - Calculates the max number of installments.
 */
function commerce_cielo_credit_card_flag_ajax_callback($form, $form_state) {

  $currency = $form_state['order']->commerce_order_total['und'][0]['currency_code'];
  $type = $form_state['values']['commerce_payment']['payment_details']['credit_card']['type'];

  if ($currency == 'BRL' && $type == Cielo::CIELO_TYPE_CREDIT_CARD) {

    $flag = $form_state['values']['commerce_payment']['payment_details']['credit_card']['flag'];
    $max_installments = $form_state['order']->payment_methods['cielo|commerce_payment_cielo']['settings']['installments'][$flag . '_max'];
    $po_total = $form_state['order']->commerce_order_total['und'][0]['amount'];

    $installment_options_array = _commerce_cielo_installment_options($po_total, $max_installments);
    $installment_options = '';
    // Construct the html tag options for Card Flag field.
    foreach ($installment_options_array as $key => $value) {
      $installment_options .= "<option value='$key'>$value</option>";
    }

    $installment_options = '<select id="edit-commerce-payment-payment-details-credit-card-installments" ' .
                              'name="commerce_payment[payment_details][credit_card][installments]"' .
                              'class="form-select">' .
                               $installment_options .
                            '</select>';

    return array(
      '#type' => 'ajax',
      '#commands' => array(
        ajax_command_replace("#edit-commerce-payment-payment-details-credit-card-installments", $installment_options),
      ),
    );
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
  $arguments = _commerce_cielo_get_arguments($payment_method, $order->order_number, $charge['amount']);

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
  $cielo = new Cielo($arguments);

  if ($payment_method['settings']['is_test']) {
    setcookie('filiacao', $cielo->membership['filiacao']);
    setcookie('chave', $cielo->membership['chave']);
  }

  // Set the currency code.
  $cielo->setCurrency($currency_numeric_code);

  // Set the language interface.
  $cielo->setLanguage($payment_method['settings']['language']);

  // Set the url for returning from Cielo.
  $url = url('checkout/' . $order->order_number . '/payment/cielo', array('absolute' => TRUE));
  $cielo->setReturnUrl($url);

  // Request Authorization.
  $cielo->authorize();

  // Check for and save any warnings that might have been issued by the external
  // library (Brazilcards).
  commerce_cielo_log_watch_dog($cielo);

  // Prepare a transaction object to log the Webservice response.
  $transaction = commerce_payment_transaction_new('cielo', $order->order_id);
  $transaction->instance_id = $payment_method['instance_id'];
  $transaction->amount = $charge['amount'];
  $transaction->currency_code = $charge['currency_code'];

  $remote_status = _commerce_cielo_get_remote_status($cielo, $payment_method['settings']['auto_capture']);

  // Save the transaction.
  commerce_cielo_save_transaction($cielo, $transaction, $authenticate, $remote_status);

  // Rebuilds the form if autorize transaction fails.
  if ($transaction->status == COMMERCE_PAYMENT_STATUS_FAILURE) {
    drupal_set_message(t('We received the following error processing your card. Please enter you information again or try a different card.'), 'error');
    drupal_set_message($transaction->message, 'error');
    return FALSE;
  }

  if (isset($cielo->response['url-autenticacao'])) {
    // Send browser to Cielo for collecting card details and/or performing
    // ownership authentication.
    drupal_goto($cielo->response['url-autenticacao']);
  }

  // CAPTURING.
  // At this point there was no redirection, so we try to capture.
  // We check if auto_capture is switched on and if there is a pending response
  // from the last call to the webservice.
  if ($payment_method['settings']['auto_capture'] == 'true' &&
      $transaction->status == Cielo::CIELO_AUTHORIZED) {
    // Try to capture.
    $cielo->capture();

    // Check for and save any warnings that might have been issued by the
    // external library (Brazilcards).
    commerce_cielo_log_watch_dog($cielo);

    // Save the transaction.
    commerce_cielo_save_transaction($cielo, $transaction, $authenticate);

    // Rebuilds the form if capture transaction fails.
    if ($transaction->status == COMMERCE_PAYMENT_STATUS_FAILURE) {
      drupal_set_message(t('We received the following error processing your card. Please enter you information again or try a different card.'), 'error');
      drupal_set_message($transaction->message, 'error');
      return FALSE;
    }
  }
}
