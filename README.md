#Instalação
* Via Drush
  * `cd sites/SEUDOMINIO`
  * `drush dl commerce_cielo`
  * `cd modules/commerce_cielo`
  * `composer install`

  * Este módulo acompanha 2 sub-módulos, são eles:
     * **commerce_cielo_checkout**:
       * `drush en -y commerce_cielo_checkout`
     * **commerce_cielo_webservice**
       * **ATENÇÃO:** NÃO ESTÁ PRONTO PARA SER USADO
       * `drush en -y commerce_cielo_webservice`

* Acesse `admin/commerce/config/cielo`, siga as instruções, informe os dados necessários e salve o formulário.

#Utilização
* **commerce_cielo_checkout** ( Redireciona o comprador para a Cielo )
  * [Clique aqui](http://developercielo.github.io/Checkout-Cielo/) para ler as instruções sobre como configurar o 
  backend da Cielo em modo de teste.
  * Acesse `admin/commerce/config/payment-methods/manage/commerce_payment_cielo_checkout/edit/3`, siga as instruções,
  informe os dados necessários e salve o formulário.
  * Você precisará implementar em seu módulo personalizado as informações do comprador e os dados do frete.
  Está disponível o hook `hook_commerce_cielo_checkout_Order_alter()` para você modificar os dados do pedido que
  serão enviados para a Cielo. Note que esse hook NÃO modifica o pedido do drupal commerce.
    * [Clique Aqui](https://github.com/drupalista-br/commerce_cielo/blob/7.x-2.x/commerce_cielo.api.php) para ver um exemplo de implementação do hook `hook_commerce_cielo_checkout_Order_alter()`.

* **commerce_cielo_webservice** ( Coleta os dados do cartão no website do vendedor )
  * **NÃO ESTÁ PRONTO PARA SER USADO**
  * Procuro interessado em ser co-mantenedor
  * [Clique aqui](https://developercielo.github.io/Webservice-1.5) para ver o manual de integração

* [Clique aqui](https://github.com/drupalista-br/CheckoutCielo-Library/blob/2.x-dev/card_numbers_for_testing.txt)
para ver lista com dados de cartões para serem usados em testes.
