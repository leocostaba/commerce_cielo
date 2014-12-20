Dependencies:
* Drupal Commerce and its dependencies.
* Payment module shipped with Drupal Commerce

Testing out on Cielo's Sandbox:

* You might want to install Commerce Kickstart. It does all the tedious settings
  for Drupal Commerce and creates some dummy products.
* Enable Commerce Cielo module normally as you would with any other module
* Enable Cielo Payment Method at admin/commerce/config/payment-methods
* Edit the configuration settings at
  admin/commerce/config/payment-methods/manage/commerce_payment_cielo
  (Click on the "Edit" operation under Actions)
* Make sure "Cielo Sandbox - Test environment" is switched on. While in there,
  you can also play around with the other configuration settings.
* You can use the following card numbers for testing on both credit and debit
  modes:
  (visa) 4012001037141112 (authenticates successfully when in debit mode)
  (visa) 4551870000000183
  (mastercard) 5453010000066167
  (elo) 6362970000457013
  Security Code: Any value (3 digits number)
  Expiration Date: Any date greater than the current date

  Authorizations are granted or denied at random.
