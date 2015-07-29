<?php

namespace CieloCheckout;

class Commons {

  public function __construct($properties = NULL) {
	if ($properties) {
	  $this->set_properties($properties);
	}
  }

  public function set_properties($properties) {
	foreach($properties as $property_name => $property_value) {
	  if (property_exists($this, $property_name)) {
		$this->$property_name = $property_value;
	  }
	}

	$this->check_requirements('properties');
  }

  private function check_requirements($requirement) {
	if (!isset($this->requirements[$requirement])) {
	  throw new \Exception("The requirement 'requirements[$requirement]' property is not set.");
	}

	switch($requirement) {
	  case 'properties':
		if (!is_array($this->requirements['properties'])) {
		  throw new \Exception("The requirement 'requirements[$requirement]' property must be an array.");
		}

		foreach($this->requirements['properties'] as $property_name) {
		  if (!property_exists($this, $property_name) || empty($this->{$property_name})) {
			throw new \Exception("The '$property_name' property is required.");
		  }
		}
	  break;
	}
  }
}
