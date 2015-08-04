<?php

/**
 * @file
 * Documents hooks provided by the Cielo modules.
 */

use
  CieloCheckout\Address,
  CieloCheckout\Shipping,
  CieloCheckout\Customer;

/**
 * Implements hook_commerce_cielo_checkout_Order_alter().
 *
 * Adds buyer and shipping details into the cielo order.
 *
 * @param Object $cielo_Order
 *   The order object which will be sent to Cielo.
 *
 * @param Array $data
 *   An array of objects containing the drupal commerce order and payment
 *   settings.
 */
function hook_commerce_cielo_checkout_Order_alter(&$cielo_Order, $data) {
  global $user;

  $profile_id = $data['order']->commerce_customer_billing[LANGUAGE_NONE][0]['profile_id'];
  $profile = commerce_customer_profile_load($profile_id);
  $address = $profile->commerce_customer_address[LANGUAGE_NONE][0];
  $city = _nortaox_user_cities()['current_selection'];
  
  // Instantiate the shipping address object.
  $properties = [
    'Street' => $address['thoroughfare'],
    'Number' => 's/n',
    'Complement' => $address['premise'],
    'District' => $address['dependent_locality'],
    'City' => $city['city_name'],
    'State' => $city['uf'],
  ];
  $cielo_Order->Shipping->Address = new Address($properties);
  $cielo_Order->Shipping->TargetZipCode = $address['postal_code'];

  // Make sure phone has only numbers.
  $phone = preg_replace('/[^0-9]+/', '', $profile->field_celular[LANGUAGE_NONE][0]['value']);
  // Make sure phone has 11 digits long by adding leading zeros.
  $phone = str_pad($phone, 11, '0', STR_PAD_LEFT);
  // Make sure phone does not exceed 11 digits long.
  $phone = substr($phone, -11);

  // Instantiate the customer object.
  $properties = [
    'Identity' => $profile->field_cpf_cnpj[LANGUAGE_NONE][0]['value'],
    'FullName' => $address['name_line'],
    'Email' => $user->mail,
    'Phone' => $phone,
  ];
  $cielo_Order->Customer = new Customer($properties);
}
