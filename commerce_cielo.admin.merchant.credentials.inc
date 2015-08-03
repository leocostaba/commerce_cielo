<?php

/**
 * @file
 * Provides the Merchant Credentials.
 */

/**
 * Form callback; merchant credentials at Cielo.
 */
function commerce_cielo_admin_merchant_credentials() {
  global $base_url;

  $form['commerce_cielo_credentials'] = array(
    '#type' => 'fieldset',
    '#title' => t('Merchant Credentials'),
    '#description' => t('The merchant affiliation ID and Secret Key given to you when you <a href="@url">sign up for a membership</a>.', array('@url' => 'https://www.cielo.com.br/sitecielo/afiliacao/credenciamentoafiliacaonaologado.html')),
  );

  $form['commerce_cielo_credentials']['commerce_cielo_merchant_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Merchant Id'),
    '#default_value' => variable_get('commerce_cielo_merchant_id', ''),
    '#maxlength' => 40,
    '#required' => TRUE,
  );
  $form['commerce_cielo_credentials']['commerce_cielo_secret_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Secret key'),
    '#default_value' => variable_get('commerce_cielo_secret_key', ''),
    '#maxlength' => 100,
    '#required' => TRUE,
  );

  $form['commerce_cielo_checkout'] = array(
    '#type' => 'fieldset',
    '#title' => t('Cielo Checkout Mode'),
  );

  $form['commerce_cielo_checkout']['commerce_cielo_checkout_return_page'] = array(
    '#type' => 'textfield',
    '#title' => t('Redirect Back Landing Page'),
    '#default_value' => variable_get('commerce_cielo_checkout_return_page', ''),
    '#description' => t('By default customers are redirected back to user/UID/orders.'),
  );

  $secret = variable_get('commerce_cielo_checkout_status_update_secret', '');
  $status_update_url = array(
    '#type' => 'textfield',
    '#title' => t('Status Update URL'),
    '#value' => "$base_url/" . COMMERCE_CIELO_CHECKOUT_STATUS_UPDATE_URL . "/$secret",
    '#disabled' => TRUE,
    '#size' => 100,
    '#description' => t('Copy and paste the above URL into both "URL de Notificação" and "URL de Mudança de Status" fields at Cielo\'s Backoffice.'),
  );

  if (empty($secret)) {
    include_once DRUPAL_ROOT . "/includes/password.inc";
    $secret = preg_replace("/[^a-zA-Z0-9]+/", '', user_hash_password(time()));
    $secret = substr(str_shuffle($secret), 0, 8);

    $status_update_url = array(
      '#type' => 'submit',
      '#value' => t('Generate Status Update URL'),
      '#suffix' => t('You will need this URL for setting up both "URL de Notificação" and "URL de Mudança de Status" fields at Cielo\'s Backoffice.'),
    );
  }

  $form['commerce_cielo_checkout']['commerce_cielo_checkout_status_update_url'] = $status_update_url;

  $form['commerce_cielo_checkout']['commerce_cielo_checkout_status_update_secret'] = array(
    '#type' => 'hidden',
    '#value' => $secret,
  );
  
  return system_settings_form($form);
}
