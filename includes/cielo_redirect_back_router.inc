<?php

/**
 * @file
 * Concludes the payment process after returning from Cielo.
 */


/**
 * ...
 */
function cielo_redirect_back($order) {
 dprint_r($order);
 
  global $user;

  if (!in_array('authenticated user', $user->roles)) {
    
    
    drupal_set_message('Please, login to conclude your checkout payment process.', 'warning');
    
    //send user to the login form
    $destination = drupal_get_destination();
    drupal_goto('user', array('query' => array('destination' => $destination['destination'])));
  }
  //making sure the po belongs to the logged in user
  if($user->uid == $order->uid){
    

    return 'testsomething';  
  }else{
    //
    drupal_set_message(t('Sorry, the order #%po does not belong to you.', array('%po' => $order->order_number)), 'error');
    drupal_goto();
  }

  
}