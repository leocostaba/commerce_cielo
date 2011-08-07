<?php

/**
 * @file
 * Concludes the payment process after returning from Cielo.
 */


/**
 * Menu callback Function for checkout/%commerce_order/payment/cielo
 */
function cielo_redirect_back($order) {
  global $user;

  if (!in_array('authenticated user', $user->roles)) {
    drupal_set_message('Please, login to conclude your checkout payment process.', 'warning');
    
    //send the user to the login form page and fill the url query value of destination with the current url path
    $destination = drupal_get_destination();
    drupal_goto('user', array('query' => array('destination' => $destination['destination'])));
  }
  //making sure the PO belongs to the current logged in user
  if($user->uid == $order->uid){
    

   
   
   
  }else{
    //interropts the flow and sends the browser to the home page
    drupal_set_message(t('Sorry, but apparently the order #%po does not belong to you. Please contact the store administrator.', array('%po' => $order->order_number)), 'error');
    drupal_goto();
  }
}