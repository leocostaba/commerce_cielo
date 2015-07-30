<?php

namespace CieloCheckout;

class Cart extends Commons {

  public
	$Discount,
	$Items;

  protected $property_requirements = [
	'Items' => [
	  'is_array' => [],
	],
  ];
  public function __construct($properties = NULL) {
	parent::__construct($properties);
	if ($properties) {
	  $this->validate();
	}
  }

  public function set_properties($properties) {
	parent::set_properties($properties);
	$this->validate();
  }

  private function validate() {
	$this->Items_validate();
  }

  private function Items_validate() {
	foreach ($this->Items as $delta => $Item) {
	  if (!$Item instanceof Item) {
		throw new \Exception("Item on index $delta of 'Items' is not an instance of Items.");
	  }
	}
  }

  protected function set_Discount(Discount $Discount) {
	$this->Discount = $Discount;
  }
}
