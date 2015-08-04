<?php

/**
 * @file
 * Listen for transaction notifications and status updates from Cielo.
 */

/**
 * Handles the transaction update POSTs done by Cielo to inform the merchant
 * website about transaction notifications and status updates.
 *
 * @todo Create a new payment transaction.
 *
 * @param String $secret_sent
 *   The URL secret argument for making sure we are communicating with Cielo.
 */
function commerce_cielo_checkout_status_update($secret_sent) {
  $secret_stored = variable_get('commerce_cielo_checkout_status_update_secret');

  //watchdog('commerce_cielo_checkout', '<pre>' . print_r($_POST, TRUE) . '</pre>');

  // Check if we are communicating with Cielo.
  if ($secret_sent == $secret_stored) {
    $amount = substr_replace($_POST['amount'], ',', -2, 0);
    watchdog(
      'commerce_cielo_checkout',
      t("Order: !order_number Cielo Order: !checkout_cielo_order_number | Status: !payment_status | Amount: !amount",
        array(
          '!order_number' => $_POST['order_number'],
          '!checkout_cielo_order_number' => $_POST['checkout_cielo_order_number'],
          '!payment_status' => $_POST['payment_status'],
          '!amount' => $amount,
        )
      )
    );

    $order = commerce_order_load($_POST['order_number']);

    $order->data['cielo_POST'][time()] = json_encode($_POST);
    $status_name = "payment_cielo_{$_POST['payment_status']}";

    commerce_order_status_update($order, $status_name, FALSE, TRUE, t('Cielo Amount: R$ !amount', array('!amount' => $amount)));
  }

  return "<status>OK</status>";
}

/**
 * Page delivery callback.
 */
function commerce_cielo_checkout_status_update_deliver_plain($page_callback_result) {
  print $page_callback_result;
}
