<?php

/**
 * @file
 * Provides the Merchant Credentials.
 */

/**
 * Form callback; merchant credentials at Cielo.
 */
function commerce_cielo_admin_merchant_credentials() {
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
  
  return system_settings_form($form);
}
