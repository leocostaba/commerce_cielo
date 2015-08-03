<?php

/**
 * @file
 * Listen for transaction notifications and status updates from Cielo.
 */

 /**
  * Handles the form POSTs done by Cielo to inform the merchant website
  * about transaction notifications and status updates.
  */
function commerce_cielo_checkout_status_update() {
  // Whether or not the posting data is from Cielo.
  $is_cielo = FALSE;

  // Whether this is the transaction notification or a status update.
  $is_status_update = empty($_POST['tid']) ? FALSE : TRUE;

  $order = commerce_order_load($_POST['order_number']);

  switch($is_status_update) {
    case TRUE:
      if (!empty($order->data['cielo_tid']) && $order->data['cielo_tid'] == trim($_POST['tid'])) {
        $is_cielo = TRUE;
      }
    break;
    case FALSE:
      if (!empty($order->data['cielo_data']['checkout_cielo_order_number']) &&
          trim($order->data['cielo_tid']['checkout_cielo_order_number']) == trim($_POST['checkout_cielo_order_number'])) {
        $is_cielo = TRUE;
      }
    break;
  }

  if ($is_cielo) {
    $order->data['cielo_data'] = $_POST;
    $status_name = "payment_cielo_{$_POST['payment_status']}";

    commerce_order_status_update($order, $status_name, FALSE, TRUE, t('Amount: !amount', array('!amount' => $_POST['amount'])));
    print "<status>OK</status>";
  }
  else {
    print "<status>ERROR</status>";
  }
}
