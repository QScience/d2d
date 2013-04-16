<?php

/**
 * Implements hook_d2d_secure_rpc().
 *
 * Some methods are associated with a default group during the installation.
 */
function d2d_d2d_secure_rpc() {
  $methods = array();
  $methods['d2d_get_public_key'] = array(
    'arguments' => array('d2d_id' => 'd2d_check_d2d_id'),
    'callback' => 'd2d_srpc_get_public_key',
    'description' => 'returns the public key of the provided instance',
  );
  $methods['d2d_list_permissions'] = array(
    'arguments' => array(),
    'callback' => 'd2d_srpc_list_permissions',
    'description' => 'returns the methods the invocing friend may call',
  );
  return $methods;
}
function d2d_srpc_get_public_key($arguments, $rpc_info) {
  $d2d_id = $arguments['d2d_id'];
  $result = db_query('SELECT p.public_key AS public_key FROM {d2d_instances} i LEFT OUTER JOIN {d2d_public_keys} p ON i.public_key_id = p.id WHERE i.d2d_id=:d2d_id', array(':d2d_id' => $d2d_id));
  if ($record = $result->fetchAssoc()) {
    return $record['public_key'] ? $record['public_key'] : '';
  }
  else {
    return '';
  }
}
function d2d_srpc_list_permissions($arguments, $rpc_info) {
  $id = $rpc_info['id'];
  $result = db_query('SELECT DISTINCT p.method as method FROM {d2d_instances} i, {d2d_group_memberships} g, {d2d_permissions} p WHERE i.id=:id AND i.id=g.instance_id AND g.group_id=p.group_id', array(':id' => $id));
  $records = $result->fetchAll();
  $allowed_methods = array();
  foreach ($records as $record) {
    $allowed_methods[$record->method] = 1;
  }
  $return_array = array();
  $hks = module_invoke_all('d2d_secure_rpc');
  foreach ($hks as $key => $value) {
    if (array_key_exists($key, $allowed_methods)) {
      $args = '?';
      if (isset($value['arguments'])) {
        $arguments = $value['arguments'];
        if (is_array($arguments)) {
          $args = var_export(array_keys($arguments), TRUE);
        }
      }
      $imploded_array = d2d_implode(
        array(
          'method' => $key,
          'description' => isset($value['description']) ? $value['description'] : '',
          'arguments' => $args,
        )
      );
      if ($imploded_array) {
        $return_array[] = $imploded_array;
      }
    }
  }
  $imploded_return_array = d2d_implode($return_array);
  if ($imploded_return_array === FALSE) {
    return '';
  }
  return $imploded_return_array;
}
