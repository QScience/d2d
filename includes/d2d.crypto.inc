<?php
/**
 * @file
 * Cryptographic functions.
 */

/**
 * Generate public / private key pair.
 *
 * @param string &$public_key
 *   The plan text public key
 *
 * @param string &$private_key
 *   The plan text private key
 *
 * @return bool
 *   TRUE, if the generation of the keys is successful
 */
function d2d_create_keys(&$public_key, &$private_key) {
  $res = @openssl_pkey_new();
  if ($res === FALSE) {
    return FALSE;
  }
  if (!@openssl_pkey_export($res, $private_key)) {
    return FALSE;
  }
  $public_key = @openssl_pkey_get_details($res);
  $public_key = $public_key['key'];
  return TRUE;
}

/**
 * Check if a public / private key pair match.
 *
 * @param string $public_key
 *   The plan text public key
 *
 * @param string $private_key
 *   The plan text private key
 *
 * @return bool
 *   TRUE, if the encryption/decryption of a test-text is successful
 */
function d2d_check_key_pair($public_key, $private_key) {
  $data = 'just a simple encryption / decryption test.';
  if (!@openssl_public_encrypt($data, $crypted, $public_key)) {
    return FALSE;
  }
  if (!@openssl_private_decrypt($crypted, $decrypted, $private_key)) {
    return FALSE;
  }
  return $decrypted === $data;
}

/**
 * Signs data.
 *
 * data string to be signed
 * 
  string plain text private key of the signer

  signer string describing the signer (probably d2d id) 
 */
function d2d_sign($data, $private_key, $signer) {
  if (@openssl_sign($data, $signature, $private_key) === FALSE) {
    return FALSE;
  }
  else {
    $array = array(
      'data' => $data,
      'signature' => $signature,
      'signer' => $signer,
    );
    return d2d_implode($array);
  }
}

/**
 * Signs data but does not include data in return.
 */
function d2d_sign2($data, $private_key, $signer) {
  if (@openssl_sign($data, $signature, $private_key) === FALSE) {
    return FALSE;
  }
  else {
    $array = array(
      'signature' => $signature,
      'signer' => $signer,
    );
    return d2d_implode($array);
  }
}

/**
 * Explodes and verifies signature string.

  string &$signature_string the return value of d2d_sign()

 signature_array exploded array on success
 */
function d2d_verify($signature_string, $public_key, &$signature_array) {
  $signature_format = array(
    'data' => 'is_string',
    'signature' => 'is_string',
    'signer' => 'is_string',
  );
  $array = d2d_explode($signature_string);
  if ($array === FALSE
      || !d2d_check_array($array, $signature_format)
      || (@openssl_verify($array['data'], $array['signature'], $public_key) !== 1)) {
    return FALSE;
  }
  $signature_array = $array;
  return TRUE;
}

/**
 * Enrypt data.

  data string to be encrypted

 
 &$encrypted_data data encrypted

 */
function d2d_encrypt($data, &$encrypted_data, $public_key) {
  if (@openssl_seal($data, $sealed_data, $env_keys, array($public_key)) === FALSE) {
    return FALSE;
  }
  $env_key = $env_keys[0];
  $tmp = d2d_implode(array('cipher' => $sealed_data, 'env_key' => $env_key));
  if ($tmp === FALSE) {
    return FALSE;
  }
  else {
    $encrypted_data = $tmp;
    return TRUE;
  }
}

/**
 * Decrypt data.
 */
function d2d_decrypt($encrypted_data, &$data, $private_key) {
  // Explode $encrypted_data.
  $cipher_envkey = d2d_explode($encrypted_data);
  if ($cipher_envkey === FALSE) {
    return FALSE;
  }
  // Check if exploded $encrypted_data is of desired structure.
  if (!d2d_check_array($cipher_envkey, array('cipher' => 'is_string', 'env_key' => 'is_string'))) {
    return FALSE;
  }
  if (!openssl_open($cipher_envkey['cipher'], $data, $cipher_envkey['env_key'], $private_key)) {
    return FALSE;
  }
  return TRUE;
}
