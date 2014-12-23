<?php

/**
 * Cielo's URL for real transactions.
 */
define('COMMERCE_CIELO_LIVE_URL', 'https://ecommerce.cielo.com.br/servicos/ecommwsec.do');

/**
 * Cielo's Sandbox URL for testing transactions.
 */
define('COMMERCE_CIELO_SANDBOX_URL', 'https://qasecommerce.cielo.com.br/servicos/ecommwsec.do');

/**
 * The membership number for when the merchants are going to collect card
 * details at their website.
 */
define('COMMERCE_CIELO_SANDBOX_ON_SITE_MERCHANT_ID', '1006993069');

/**
 * The token for when the merchants are going to collect card details at their
 * website.
 */
define('COMMERCE_CIELO_SANDBOX_ON_SITE_MERCHANT_TOKEN', '25fbb99741c739dd84d7b06ec78c9bac718838630f30b112d033ce2e621b34f3');

/**
 * The membership number for when the merchants redirect customers to
 * Cielo's website where the card details are going to be collected.
 */
define('COMMERCE_CIELO_SANDBOX_OFF_SITE_MERCHANT_ID', '1001734898');

/**
 * The token for when the merchants redirect customers to Cielo's website
 * where the card details are going to be collected.
 */
define('COMMERCE_CIELO_SANDBOX_OFF_SITE_MERCHANT_TOKEN', 'e84827130b9837473681c2787007da5914d6359947015a5cdb2b8843db0fa832');

/**
 * Cielo message version.
 */
define('COMMERCE_CIELO_MESSAGE_VERSION', '1.4.0');

/**
 * Cielo status: transaction created.
 */
define('COMMERCE_CIELO_CREATED', 0);

/**
 * Cielo status: transaction in progress.
 */
define('COMMERCE_CIELO_IN_PROGRESS', 1);

/**
 * Cielo status: transaction authenticated.
 */
define('COMMERCE_CIELO_AUTHENTICATED', 2);

/**
 * Cielo status: transaction not authenticated.
 */
define('COMMERCE_CIELO_NOT_AUTHENTICATED', 3);

/**
 * Cielo status: transaction authorized.
 */
define('COMMERCE_CIELO_AUTHORIZED', 4);

/**
 * Cielo status: transaction not authorized.
 */
define('COMMERCE_CIELO_NOT_AUTHORIZED', 5);

/**
 * Cielo status: transaction captured.
 */
define('COMMERCE_CIELO_CAPTURED', 6);

/**
 * Cielo status: transaction canceled.
 */
define('COMMERCE_CIELO_CANCELED', 9);

/**
 * Cielo status: transaction being authenticated.
 */
define('COMMERCE_CIELO_AUTHENTICATING', 10);

/**
 * Cielo language: Portuguese.
 */
define('COMMERCE_CIELO_LANG_PT', 'PT');

/**
 * Cielo language:Spanish.
 */
define('COMMERCE_CIELO_LANG_ES', 'ES');

/**
 * Cielo language: English.
 */
define('COMMERCE_CIELO_LANG_EN', 'EN');

/**
 * Authorization mode: authenticate only.
 */
define('COMMERCE_CIELO_AUTHORIZATION_AUTHENTICATION_ONLY', 0);

/**
 * Authorization mode: authorize if authenticated.
 */
define('COMMERCE_CIELO_AUTHORIZATION_AUTHORIZE_AUTHENTICATED', 1);

/**
 * Authorization mode: authorize if authenticated or not.
 */
define('COMMERCE_CIELO_AUTHORIZATION_AUTHORIZE', 2);

/**
 * Authorization mode: authorize skiping authentication.
 */
define('COMMERCE_CIELO_AUTHORIZATION_AUTHORIZE_SKIP_AUTHENTICATION', 3);

/**
 * Authorization mode: recurring transaction.
 */
define('COMMERCE_CIELO_AUTHORIZATION_RECURRING', 4);

/**
 * Cielo card type: Visa.
 */
define('COMMERCE_CIELO_CARD_VISA', 'visa');

/**
 * Cielo card type: MasterCard.
 */
define('COMMERCE_CIELO_CARD_MASTERCARD', 'mastercard');

/**
 * Cielo card type: Diners Club.
 */
define('COMMERCE_CIELO_CARD_DINERS', 'diners');

/**
 * Cielo card type: Discover.
 */
define('COMMERCE_CIELO_CARD_DISCOVER', 'discover');

/**
 * Cielo card type: Elo.
 */
define('COMMERCE_CIELO_CARD_ELO', 'elo');

/**
 * Cielo card type: American Express.
 */
define('COMMERCE_CIELO_CARD_AMEX', 'amex');

/**
 * Cielo card type: JCB.
 */
define('COMMERCE_CIELO_CARD_JCB', 'jcb');

/**
 * Cielo card type: Aura.
 */
define('COMMERCE_CIELO_CARD_AURA', 'aura');

/**
 * Payment method: debit.
 */
define('COMMERCE_CIELO_METHOD_DEBIT', 'A');

/**
 * Payment method: credit.
 */
define('COMMERCE_CIELO_METHOD_CREDIT', 1);

/**
 * Payment method: credit.
 */
define('COMMERCE_CIELO_METHOD_CREDIT_INSTALLMENTS', 2);

/**
 * Authorize a payment.
 */
function commerce_cielo_authorize($settings, $values, $order, $charge) {
  watchdog('commerce_cielo', 'Settings: ' . print_r($settings, TRUE));
  watchdog('commerce_cielo', 'Values: ' . print_r($values, TRUE));
  watchdog('commerce_cielo', 'Charge: ' . print_r($charge, TRUE));

  $credentials = commerce_cielo_credentials($settings);
  $xml = commerce_cielo_create_envelope('requisicao-transacao', $credentials['affiliation'], $credentials['token']);

  if ($settings['collect_card_details']) {
    // Card information.
    $card_info =& $xml->{'dados-portador'};
    $card_info->numero = $values['credit_card']['number'];
    $card_info->validade = $values['credit_card']['exp_year'] . $values['credit_card']['exp_month'];
    $card_info->indicador = 1;
    $card_info->{'codigo-seguranca'} = $values['credit_card']['code'];
  }

  // Order information.
  $order_info =& $xml->{'dados-pedido'};
  $order_info->numero = $order->order_id;
  $order_info->valor = $charge['amount'];
  $order_info->moeda = commerce_cielo_currency_code($charge['currency_code']);
  $order_info->{'data-hora'} = date("Y-m-d\TH:i:s", REQUEST_TIME);
  $order_info->idioma = $settings['language'];

  // Payment information.
  $payment_info =& $xml->{'forma-pagamento'};
  $payment_info->bandeira = $values['credit_card']['flag'];
  $payment_info->produto = $values['credit_card']['type'];
  $payment_info->parcelas = $values['credit_card']['installments'];

  // Transaction options.
  $xml->{'url-retorno'} = url('checkout/' . $order->order_number . '/payment/cielo', array('absolute' => TRUE));
  $xml->autorizar = $settings['authorization_type'];
  $xml->capturar = $settings['auto_capture'];

  watchdog('commerce_cielo', 'Envelope: ' . $xml->saveXML());

  return commerce_cielo_request($xml->saveXML(), $settings['is_test']);
}

/**
 * Check status of a transaction.
 */
function commerce_cielo_check_status($settings, $transaction) {
  $credentials = commerce_cielo_credentials($settings);
  $xml = commerce_cielo_create_envelope('requisicao-consulta', $credentials['affiliation'], $credentials['token']);
  $xml->tid = $transaction->remote_status;

  watchdog('commerce_cielo', 'Envelope check_status: ' . $xml->saveXML());

  return commerce_cielo_request($xml->saveXML(), $settings['is_test']);
}

/**
 * Capture a payment.
 */
function commerce_cielo_capture($settings, $transaction) {
  // @TODO: Build XML envelope and call Cielo webservice.
}

function commerce_cielo_create_envelope($type, $affiliation, $token) {
  $xml = new SimpleXMLElement('<?xml version="1.0" encoding="ISO-8859-1"?><' . $type . ' id="' . uuid_generate() . '" versao="' . COMMERCE_CIELO_MESSAGE_VERSION . '" />');

  // Request authenticating node.
  $auth =& $xml->{'dados-ec'};
  $auth->numero = $affiliation;
  $auth->chave = $token;

  return $xml;
}

/**
 * Convert currency code to the ISO 4217 (numeric code).
 */
function commerce_cielo_currency_code($code) {
  // Get the ISO 4217 info about all the enabled currencies.
  $currencies = commerce_currencies(TRUE);

  // Get the currency code formatted as 3 digits number. ie. 840 for USD.
  foreach ($currencies as $tree_letter_code => $value) {
    if ($tree_letter_code == $code) {
      $currency_numeric_code = $value['numeric_code'];
      break;
    }
  }

  return $currency_numeric_code;
}

/**
 * Get credentials for authenticating the webservice request.
 */
function commerce_cielo_credentials($settings) {
  if ($settings['is_test']) {
    return array(
      'affiliation' => $settings['collect_card_details'] ? COMMERCE_CIELO_SANDBOX_ON_SITE_MERCHANT_ID : COMMERCE_CIELO_SANDBOX_OFF_SITE_MERCHANT_ID,
      'token' => $settings['collect_card_details'] ? COMMERCE_CIELO_SANDBOX_ON_SITE_MERCHANT_TOKEN : COMMERCE_CIELO_SANDBOX_OFF_SITE_MERCHANT_TOKEN,
    );
  }

  return $settings['credentials'];
}

/**
 * Call Cielo XML webservice.
 */
function commerce_cielo_request($xml, $test = FALSE) {
  watchdog('commerce_cielo', 'Test: ' . $test);
  $curl = curl_init();

  curl_setopt($curl, CURLOPT_URL, $test ? COMMERCE_CIELO_SANDBOX_URL : COMMERCE_CIELO_LIVE_URL);
  curl_setopt($curl, CURLOPT_FAILONERROR, TRUE);
  curl_setopt($curl, CURLOPT_SSL_VERIFYPEER, !$test);
  curl_setopt($curl, CURLOPT_SSL_VERIFYHOST, 2);

  if (!$test) {
    curl_setopt($curl, CURLOPT_CAINFO, drupal_get_path('module', 'commerce_cielo') . '/VeriSignClass3PublicPrimaryCertificationAuthority-G5.crt');
  }

  curl_setopt($curl, CURLOPT_SSLVERSION, 3);
  curl_setopt($curl, CURLOPT_CONNECTTIMEOUT, 10);
  curl_setopt($curl, CURLOPT_TIMEOUT, 40);
  curl_setopt($curl, CURLOPT_RETURNTRANSFER, TRUE);
  curl_setopt($curl, CURLOPT_POST, TRUE);
  curl_setopt($curl, CURLOPT_POSTFIELDS, 'mensagem=' . $xml);

  $result = curl_exec($curl);

  watchdog('commerce_cielo', 'Result: ' . print_r($result, TRUE));

  if (!$result) {
    // TODO: log CURL errors.
    return FALSE;
  }

  // TODO: parse response.
  $response = simplexml_load_string($result);

  curl_close($curl);

  return $response;
}