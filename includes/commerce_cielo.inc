<?php

/**
 * Cielo's URL for real transactions.
 */
define('COMMERCE_CIELO_LIVE_URL', 'https://ecommerce.cielo.com.br/servicos/ecommwsec.do');

/**
 * Cielo's Sandbox URL for testing transactions.
 */
define('COMMERCE_CIELO_SANDBOX_URL', 'https://qasecommerce.cielo.com.br/servicos/ecommwsec.do');

/**
 * The membership number for when the merchants are going to collect card
 * details at their website.
 */
define('COMMERCE_CIELO_SANDBOX_ON_SITE_MERCHANT_ID', '1006993069');

/**
 * The token for when the merchants are going to collect card details at their
 * website.
 */
define('COMMERCE_CIELO_SANDBOX_ON_SITE_MERCHANT_TOKEN', '25fbb99741c739dd84d7b06ec78c9bac718838630f30b112d033ce2e621b34f3');

/**
 * The membership number for when the merchants redirect customers to
 * Cielo's website where the card details are going to be collected.
 */
define('COMMERCE_CIELO_SANDBOX_OFF_SITE_MERCHANT_ID', '1001734898');

/**
 * The token for when the merchants redirect customers to Cielo's website
 * where the card details are going to be collected.
 */
define('COMMERCE_CIELO_SANDBOX_OFF_SITE_MERCHANT_TOKEN', 'e84827130b9837473681c2787007da5914d6359947015a5cdb2b8843db0fa832');


/**
 * Cielo status: transaction created.
 */
define('COMMERCE_CIELO_TRANSACTION_CREATED', 0);

/**
 * Cielo status: transaction in progress.
 */
define('COMMERCE_CIELO_IN_PROGRESS', 1);

/**
 * Cielo status: transaction authenticated.
 */
define('COMMERCE_CIELO_AUTHENTICATED', 2);

/**
 * Cielo status: transaction not authenticated.
 */
define('COMMERCE_CIELO_NOT_AUTHENTICATED', 3);

/**
 * Cielo status: transaction authorized.
 */
define('COMMERCE_CIELO_AUTHORIZED', 4);

/**
 * Cielo status: transaction not authorized.
 */
define('COMMERCE_CIELO_NOT_AUTHORIZED', 5);

/**
 * Cielo status: transaction captured.
 */
define('COMMERCE_CIELO_CAPTURED', 6);

/**
 * Cielo status: transaction canceled.
 */
define('COMMERCE_CIELO_CANCELED', 9);

/**
 * Cielo status: transaction being authenticated.
 */
define('COMMERCE_CIELO_AUTHENTICATING', 10);

/**
 * Cielo language: Portuguese.
 */
define('COMMERCE_CIELO_LANG_PT', 'PT');

/**
 * Cielo language:Spanish.
 */
define('COMMERCE_CIELO_LANG_ES', 'ES');

/**
 * Cielo language: English.
 */
define('COMMERCE_CIELO_LANG_EN', 'EN');

/**
 * Authorization mode: authenticate only.
 */
define('COMMERCE_CIELO_AUTHORIZATION_AUTHENTICATION_ONLY', 0);

/**
 * Authorization mode: authorize if authenticated.
 */
define('COMMERCE_CIELO_AUTHORIZATION_AUTHORIZE_AUTHENTICATED', 1);

/**
 * Authorization mode: authorize if authenticated or not.
 */
define('COMMERCE_CIELO_AUTHORIZATION_AUTHORIZE', 2);

/**
 * Authorization mode: authorize skiping authentication.
 */
define('COMMERCE_CIELO_AUTHORIZATION_AUTHORIZE_SKIP_AUTHENTICATION', 3);

/**
 * Authorization mode: recurring transaction.
 */
define('COMMERCE_CIELO_AUTHORIZATION_RECURRING', 4);

/**
 * Cielo card type: Visa.
 */
define('COMMERCE_CIELO_CARD_VISA', 'visa');

/**
 * Cielo card type: MasterCard.
 */
define('COMMERCE_CIELO_CARD_MASTERCARD', 'mastercard');

/**
 * Cielo card type: Diners Club.
 */
define('COMMERCE_CIELO_CARD_DINERS', 'diners');

/**
 * Cielo card type: Discover.
 */
define('COMMERCE_CIELO_CARD_DISCOVER', 'discover');

/**
 * Cielo card type: Elo.
 */
define('COMMERCE_CIELO_CARD_ELO', 'elo');

/**
 * Cielo card type: American Express.
 */
define('COMMERCE_CIELO_CARD_AMEX', 'amex');

/**
 * Cielo card type: JCB.
 */
define('COMMERCE_CIELO_CARD_JCB', 'jcb');

/**
 * Cielo card type: Aura.
 */
define('COMMERCE_CIELO_CARD_AURA', 'aura');

/**
 * Payment method: debit.
 */
define('COMMERCE_CIELO_METHOD_DEBIT', 'A');

/**
 * Payment method: credit.
 */
define('COMMERCE_CIELO_METHOD_CREDIT', 1);

/**
 * Payment method: credit.
 */
define('COMMERCE_CIELO_METHOD_CREDIT_INSTALLMENTS', 2);
