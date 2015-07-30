<?php

namespace CieloCheckout;

class Order extends Commons {

  public
    /**
	 * Merchant's order number.
	 */
	$OrderNumber,

    /**
	 * Text shown on the buyer's bill right after merchant's name.
	 */
	$SoftDescriptor,
	$Cart,
	$Shipping,
	$Payment,
	$Customer,
	$Options;

  protected $property_requirements = [
	'Cart' => [
	  'empty' => ['negate' => FALSE],
	],
	'Shipping' => [
	  'empty' => ['negate' => FALSE],
	],
  ];

  protected function set_Cart(Cart $Cart) {
	$this->Cart = $Cart;
  }
}
