<?php

// TODO add _patterns and update methods calls

function d2d_api_public_key_exist($d2d_id, $public_key) {
  return d2d_api_public_key_return_id($d2d_id, $public_key) !== FALSE;
}

// TODO: $d2d_id -> $id, all function in the D2D-module have the id
//  and not just the $d2d_id available
// STE: CHANGE
function d2d_api_public_key_set($d2d_id, $public_key) {
  $instance_id = d2d_convert_d2d_id_to_id($d2d_id);
  $id_pk = db_insert('d2d_public_keys')->fields(array(
      'instance_id' => $instance_id,
      'public_key' => $public_key,
  ))->execute();
  return $id_pk;
}

// TODO: used by d2d_patterns only, can possibly break things
//   variable_del has no return value, there should be no method
//   to delete the own public key without deleting the private
//   key
function d2d_api_own_public_key_del() {

  $num_deleted = db_delete('d2d_public_keys')
  ->condition('id_instance', $id)
  ->execute();
   
  variable_del('d2d_public_key');

}