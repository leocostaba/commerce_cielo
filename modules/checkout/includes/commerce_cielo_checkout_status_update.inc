<?php

/**
 * @file
 * Listen for transaction notifications and status updates from Cielo.
 */

 /**
  * Handles the transaction update POSTs done by Cielo to inform the merchant
  * website about transaction notifications and status updates.
  *
  * @param String $secret_sent
  *   The URL secret argument for making sure we are communicating with Cielo.
  */
function commerce_cielo_checkout_status_update($secret_sent) {
  $secret_stored = variable_get('commerce_cielo_checkout_status_update_secret');

  // Check if we are communicating with Cielo.
  if ($secret_sent == $secret_stored) {
    $amount = substr_replace($_POST['amount'], ',', -2, 0);

    $order = commerce_order_load($_POST['order_number']);

    $order->data['cielo_POST'][time()] = json_encode($_POST);
    $status_name = "payment_cielo_{$_POST['payment_status']}";

    commerce_order_status_update($order, $status_name, FALSE, TRUE, t('Cielo Amount: R$ !amount', array('!amount' => $amount)));
  }

  print "<status>OK</status>";
}
