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
  print "<status>OK</status>";
}
