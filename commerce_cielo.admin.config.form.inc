<?php
/**
 * @file
 * Commerce Cielo payment method backend configuration form.
 */

/**
 * Payment method callback: settings form.
 */
function commerce_cielo_settings_form($settings = NULL) {

  // System permission.
  if (!user_access('administer commerce cielo')) {
    return drupal_access_denied();
  }

  $form = array();

  // Merge default settings into the stored settings array.
  $settings = (array) $settings + array(
    'is_test' => 0,
    'credentials' => array(
      // filiacao.
      'affiliation' => '',
      // chave.
      'token' => '',
    ),
    'language' => COMMERCE_CIELO_LANG_EN,
    'authorization_type' => COMMERCE_CIELO_AUTHORIZATION_AUTHORIZE,
    'collect_card_details' => TRUE,
    'authenticate' => TRUE,

    // Bank and card mode specific settings.
    'tuning' => array(
      'debit_mode_flag_support' => array_keys(commerce_cielo_card_types(TRUE)),
      'credit_mode_flag_support' => array_keys(commerce_cielo_card_types()),
      // @see commerce_cielo_bank_codes().
      'debit_mode_suported_banks' => '237',
    ),

    'installments' => array(),
    'auto_capture' => 'true',
  );

  // Set default max number of installments for each card type.
  foreach(array_keys(commerce_cielo_card_types()) as $type_name) {
    // Discover doesn't support installments.
    if ($type_name != COMMERCE_CIELO_CARD_DISCOVER && empty($settings['installments'][$type_name . '_max']))  {
      $settings['installments'][$type_name . '_max'] = 18;
    }
  }

  $form['is_test'] = array(
    '#type' => 'radios',
    '#title' => t('Live / Sandbox'),
    '#description' => t('Define either if this is a Sandbox enviroment for testing out the webservice or a Live Production Webservice.'),
    '#options' => array(
      FALSE => t('Live Webservice - Real Transactions'),
      TRUE => t('Cielo Sandbox - Test enviroment'),
    ),
    '#ajax' => array(
      'callback' => 'commerce_cielo_credentials_ajax_callback',
      'wrapper' => 'credentials_wrapper',
    ),
    '#default_value' => $settings['is_test'],
  );

  $credentials_required = FALSE;
  $credentials_display = 'block';
  if ($settings['is_test']) {
    // If is test then hide the live credential fields from view.
    $credentials_display = 'none';
  }
  else {
    // If is live then make the live credential fields required.
    $credentials_required = TRUE;
  }

  $form['credentials'] = array(
    '#type' => 'fieldset',
    '#title' => t('Webservice Credentials at CIELO'),
    '#prefix' => "<div id='credentials_wrapper'>",
    '#suffix' => '</div>',
    '#attributes' => array('style' => array("display:$credentials_display")),
  );

  $form['credentials']['affiliation'] = array(
    '#type' => 'textfield',
    '#title' => t('Affiliation ID'),
    '#description' => t('This value (filiacao) is provided by Cielo up on subscription.'),
    '#default_value' => $settings['credentials']['affiliation'],
    '#required' => $credentials_required,
  );

  $form['credentials']['token'] = array(
    '#type' => 'textfield',
    '#title' => t('Token Key'),
    '#description' => t('This value (chave) is provided by Cielo up on subscription.'),
    '#default_value' => $settings['credentials']['token'],
    '#required' => $credentials_required,
  );

  $form['language'] = array(
    '#type' => 'select',
    '#title' => t("Cielo's Language Interface"),
    '#description' => t('Set the language in which Cielo will provide its webservice responses as well as its user interface for when a client redirection to Cielo is required.'),
    '#options' => array(
      COMMERCE_CIELO_LANG_PT => t('Portuguese'),
      COMMERCE_CIELO_LANG_ES => t('Spanish'),
      COMMERCE_CIELO_LANG_EN => t('English'),
    ),
    '#default_value' => $settings['language'],
  );

  $form['auto_capture'] = array(
    '#type' => 'radios',
    '#title' => t('Capturing Funds'),
    '#description' => t('Define whether or not to automatically capture the order total amount up on authorization granting.'),
    '#options' => array(
      'false' => t('Authorization only (requires manual capturing after checkout)'),
      'true' => t('Authorization and capture'),
    ),
    '#default_value' => $settings['auto_capture'],
  );

  $form['authorization_type'] = array(
    '#type' => 'radios',
    '#title' => t('Authorization Mode'),
    '#description' => t('Define how the card holder authentication will be handled.'),
    '#options' => array(
      // @todo: we might not need all transactions types.
      COMMERCE_CIELO_AUTHORIZATION_AUTHENTICATION_ONLY => t("Don't authorize (only authenticate)"),
      COMMERCE_CIELO_AUTHORIZATION_AUTHORIZE_AUTHENTICATED => t('Authorize only if authenticated'),
      COMMERCE_CIELO_AUTHORIZATION_AUTHORIZE => t('Authorize if authenticated or not'),
      COMMERCE_CIELO_AUTHORIZATION_AUTHORIZE_SKIP_AUTHENTICATION => t('Authorize skiping authentication'),
      COMMERCE_CIELO_AUTHORIZATION_RECURRING => t('Recurring transaction'),
    ),
    '#default_value' => $settings['authorization_type'],
  );

  // Added AJAX callback.
  $form['collect_card_details'] = array(
    '#type' => 'radios',
    '#title' => t('Where do buyers hand over their card details?'),
    '#options' => array(
      t("At Cielo. Buyers are redirected to Cielo's Website."),
      t("At Merchant. Buyers enter their card details at the merchant's Website."),
    ),
    '#ajax' => array(
      'callback' => 'commerce_cielo_collect_card_details_ajax_callback',
      'wrapper' => 'authenticate_wrapper',
    ),
    '#default_value' => $settings['collect_card_details'],
  );

  // Setting an empty element with a wrapper for ajax use.
  $form['authenticate'] = array(
    '#type' => 'markup',
    '#prefix' => '<div id="authenticate_wrapper">',
    '#suffix' => '</div>',
  );

  if ($settings['collect_card_details']) {
    $form['authenticate']['#type'] = 'select';
    $form['authenticate']['#title'] = t('Should the card holder be authenticated?');
    $form['authenticate']['#description'] = t('This applies on for when Credit card mode type is selected. For Debit cards this will be always YES. Define whether or not the card holder (buyer) will be prompted for card ownership verification.');
    $form['authenticate']['#options'] = array(
      FALSE => t('No'),
      TRUE => t('Yes'),
    );
    $form['authenticate']['#default_value'] = $settings['authenticate'];
  }

  $form['tuning'] = array(
    '#type' => 'fieldset',
    '#title' => t('TUNING'),
    '#description' => t('Bank and card mode specific settings.'),
  );

  $form['tuning']['debit_mode_flag_support'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Debit mode flag support'),
    '#description' => t('Define which flags the debit mode supports.'),
    '#options' => commerce_cielo_card_types(TRUE),
    '#default_value' => $settings['tuning']['debit_mode_flag_support'],
  );

  $form['tuning']['credit_mode_flag_support'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Credit mode flag support'),
    '#description' => t('Define which flags the credit mode supports.'),
    '#options' => commerce_cielo_card_types(),
    '#default_value' => $settings['tuning']['credit_mode_flag_support'],
  );

  $form['installments'] = array(
    '#type' => 'fieldset',
    '#title' => t('INSTALLMENT SETTINGS'),
    '#description' => t('These settings are ignored when the purchase order currency is other than BRL.'),
  );

  foreach(commerce_cielo_card_types() as $type_name => $type) {
    // Discover doesn't support installments.
    if ($type_name == COMMERCE_CIELO_CARD_DISCOVER) {
      continue;
    }

    $form['installments'][$type_name . '_max'] = array(
      '#type' => 'textfield',
      '#title' => t('Max number of installments for @card_type', array('@card_type' => $type)),
      '#description' => t('Defines the max number of installments that a buyer can select at the check out pane when paying with a @card_type Card.', array('@card_type' => $type)),
      '#size' => 2,
      '#default_value' => $settings['installments'][$type_name . '_max'],
      '#element_validate' => array('element_validate_integer_positive'),
    );
  }

  return $form;
}

/**
 * AJAX callback for collect_card_details.
 */
function commerce_cielo_credentials_ajax_callback($form, $form_state) {

  $settings = $form_state['values']['parameter']['payment_method']['settings']['payment_method']['settings'];

  $credentials_required = FALSE;
  $credentials_display = 'block';
  if ($settings['is_test']) {
    // If it is test then hide the live credential fields from view.
    $credentials_display = 'none';
  }
  else {
    // If it is live then make the live credential fields required.
    $credentials_required = TRUE;
  }

  $form['credentials'] = array(
    '#type' => 'fieldset',
    '#title' => t('Webservice Credentials at CIELO'),
    '#prefix' => "<div id='credentials_wrapper'>",
    '#suffix' => '</div>',
    '#attributes' => array('style' => array("display:$credentials_display")),
  );
  $form['credentials']['affiliation'] = array(
    '#type' => 'textfield',
    '#title' => t('Affiliation ID'),
    '#description' => t('This value (filiacao) is provided by Cielo up on subscription.'),
    '#value' => $settings['credentials']['affiliation'],
    '#required' => $credentials_required,
  );

  $form['credentials']['token'] = array(
    '#type' => 'textfield',
    '#title' => t('Token Key'),
    '#description' => t('This value (chave) is provided by Cielo up on subscription.'),
    '#value' => $settings['credentials']['token'],
    '#required' => $credentials_required,
  );

  return $form['credentials'];
}

/**
 * AJAX callback for collect_card_details.
 */
function commerce_cielo_collect_card_details_ajax_callback($form, $form_state) {

  // Reconstruct markup.
  $form['authenticate'] = array(
    '#type' => 'markup',
    '#prefix' => '<div id="authenticate_wrapper">',
    '#suffix' => '</div>',
  );

  if ($form_state['values']['parameter']['payment_method']['settings']['payment_method']['settings']['collect_card_details']) {
    // Replace markup.
    $form['authenticate'] = array(
      '#type' => 'select',
      '#title' => t('Should the card holder be authenticated?'),
      '#description' => t('Define whether or not the card holder (buyer) will be prompted for card ownership verification.'),
      '#options' => array(
        FALSE => t('No.'),
        TRUE => t('Yes.'),
      ),
      '#prefix' => '<div id="authenticate_wrapper">',
      '#suffix' => '</div>',
    );
  }

  return $form['authenticate'];
}
