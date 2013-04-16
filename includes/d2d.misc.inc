<?php
/**
 * @file
 * Miscellaneous helper functions.
 */

/**
 * Return the version number of the module as a string.
 */
function d2d_get_version() {
  return D2D_VERSION_STRING;
}

/**
 * Returns the options for an XML-RPC request, i.e. the third argument given
 * to xmlrpc(...)
 */
function d2d_xmlrpc_options() {
  return array(
    'timeout' => variable_get('d2d_timeout', D2D_TIMEOUT_DEFAULT),
  );
}

/**
 * Returns whether this D2D-instance is online.
 */
function d2d_is_online() {
  return variable_get('d2d_online', FALSE);
}

function d2d_get_time() {
  return time() + variable_get('d2d_utc_offset', D2D_UTC_OFFSET_DEFAULT);
}

// TODO: use t(.) for units
function d2d_format_unit($v, $unit, $skip=FALSE) {
  if (!$v && $skip) {
    return '';
  }
  else {
    return strval($v) . ' ' . $unit . ($v != 1 ? 's' : '');
  }
}
function d2d_get_instance_by_d2d_id($d2d_id) {
  $result = db_query('SELECT p.public_key as public_key, i.id as id, i.name as name, i.url as url, i.description as description, i.friendship_state as friendship_state FROM {d2d_instances} i LEFT OUTER JOIN {d2d_public_keys} p ON i.public_key_id = p.id WHERE i.d2d_id=:d2d_id', array(':d2d_id' => $d2d_id));
  if ($row = $result->fetchAssoc()) {
    $row['d2d_id'] = $d2d_id;
    $row['is_friend'] = ($row['friendship_state'] == 3);
    return $row;
  }
  else {
    return FALSE;
  }
}

function d2d_get_instance2($id) {
  $result = db_query('SELECT p.public_key as public_key, i.d2d_id as d2d_id, i.name as name, i.url as url, i.description as description, i.friendship_state as friendship_state FROM {d2d_instances} i LEFT OUTER JOIN {d2d_public_keys} p ON i.public_key_id = p.id WHERE i.id=:id', array(':id' => $id));
  if ($row = $result->fetchAssoc()) {
    $row['id'] = $id;
    $row['is_friend'] = ($row['friendship_state'] == 3);
    return $row;
  }
  else {
    return FALSE;
  }
}
function d2d_get_self_instance2() {
  $id = variable_get('d2d_my_id');
  return d2d_get_instance2($id);
}

function d2d_get_self_instance(&$id, &$d2d_id, &$url, &$public_key) {
  $id = variable_get('d2d_my_id');
  $result = db_query('SELECT p.public_key as public_key, i.d2d_id as d2d_id, i.url as url FROM {d2d_instances} i LEFT OUTER JOIN {d2d_public_keys} p ON i.public_key_id = p.id WHERE i.id=:id', array(':id' => $id));
  if ($row = $result->fetchAssoc()) {
    $url = $row['url'];
    $public_key = $row['public_key'];
    $d2d_id = $row['d2d_id'];
  }
  else {
    return FALSE;
  }
}

// TODO: localization
function d2d_get_absolute_time($t) {
  return date('F j, Y, g:i a', $t);
}

function d2d_check_url($url) {
  if (!is_string($url)) {
    return FALSE;
  }
  if (!preg_match('/^https?:\/\/.+$/', $url)) {
    return FALSE;
  }
  return strlen($url) <= D2D_INSTANCE_URL_MAX_LENGTH;
}

function d2d_get_relative_time($t) {
  $diff = $t - d2d_get_time();
  $prefix = '';
  $suffix = '';
  if ($diff > 0) {
    $absdiff = $diff;
    $prefix = 'in ';
  }
  else {
    $absdiff = -$diff;
    $suffix = ' ago';
  }
  $diffs = array(
    gmdate('Y', $absdiff) - 1970,
    gmdate('n', $absdiff) - 1,
    gmdate('j', $absdiff) - 1,
    gmdate('G', $absdiff),
    intval(gmdate('i', $absdiff)),
    intval(gmdate('s', $absdiff)),
    0,
  );
  $relative = '';
  $units = array('year', 'month', 'day', 'hour', 'minute', 'second', '');
  for ($i = 0; $i < 6; $i++) {
    if ($i == 5 || $diffs[$i] > 0) {
      $relative = d2d_format_unit($diffs[$i], $units[$i]);
      $tmp = d2d_format_unit($diffs[$i + 1], $units[$i + 1], TRUE);
      if ($tmp) {
        $relative = "$relative, $tmp";
      }
      break;
    }
  }
  return $prefix . $relative . $suffix;
}

function d2d_bool_to_str($b) {
  if (is_null($b)) {
    return t('may be');
  }
  else {
    return $b ? t('true') : t('false');
  }
}

/**
 * Return string stating friendship
 */
function d2d_get_friendship_string($signer, $friend, $until) {
  return d2d_implode(
    array(
      'type' => 'friendship',
      'signer' => $signer,
      'friend' => $friend,
      'until' => $until,
    )
  );
}

function d2d_get_request_type($i) {
  switch ($i) {
    case 0:
      return 'request';
    case 1:
      return 'accept';
    case 2:
      return 'renew';
    default:
      return 'N/A';
  }
}


function d2d_get_friends() {
  $result = db_query('SELECT i.id as id, i.d2d_id as d2d_id, i.name as name, i.url as url, i.description as description, i.time_inserted as time_inserted, i.last_alive as last_alive, p.public_key as public_key FROM {d2d_instances} i LEFT OUTER JOIN {d2d_public_keys} p ON i.public_key_id = p.id WHERE i.friendship_state=3');
  $friends = array();
  foreach ($result as $record) {
    $friends[$record->d2d_id] = array(
      'id' => $record->id,
      'd2d_id' => $record->d2d_id,
      'name' => $record->name,
      'url' => $record->url,
      'description' => $record->description,
      'time_inserted' => $record->time_inserted,
      'last_alive' => $record->last_alive,
      'public_key' => $record->public_key,
    );
  }
  return $friends;
}

function d2d_get_instances($d2d_id_as_key) {
  $result = db_query('SELECT i.id as id, i.d2d_id as d2d_id, i.name as name, i.url as url, i.description as description, i.time_inserted as time_inserted, i.last_alive as last_alive, i.friendship_state as friendship_state, p.public_key as public_key FROM {d2d_instances} i LEFT OUTER JOIN {d2d_public_keys} p ON i.public_key_id = p.id ORDER BY i.id ASC');
  $my_id = variable_get('d2d_my_id');
  $instances = array();
  foreach ($result as $record) {
    $instances[$d2d_id_as_key ? $record->d2d_id : $record->id] = array(
      'id' => $record->id,
      'd2d_id' => $record->d2d_id,
      'name' => $record->name,
      'url' => $record->url,
      'description' => $record->description,
      'time_inserted' => $record->time_inserted,
      'last_alive' => $record->last_alive,
      'public_key' => $record->public_key,
      'friendship_state' => $record->friendship_state,
      'is_friend' => ($record->friendship_state == 3),
      'is_me' => ($my_id === $record->id),
    );
  }
  return $instances;
}

/**
 * Include the instance with the provided ID in all groups that are marked as default.
 */
function d2d_set_default_group_memberships($id) {
  $result = db_query('SELECT id FROM {d2d_groups} WHERE auto_member=1');
  $records = $result->fetchAll();
  if (!empty($records)) {
    db_delete('d2d_group_memberships')
      ->condition('instance_id', $id)->execute();
    $query = db_insert('d2d_group_memberships')->fields(array('group_id', 'instance_id'));
    foreach ($records as $gid) {
      $query->values(array('group_id' => $gid->id, 'instance_id' => $id));
    }
    $query->execute();
  }
}
function d2d_verify_timestamp($current_time, $timestamp) {
  return ($timestamp >= $current_time - variable_get('d2d_delta_0', D2D_DELTA_0_DEFAULT) && $timestamp <= $current_time + variable_get('d2d_delta_1', D2D_DELTA_1_DEFAULT));
}
function d2d_verify_nonce($timestamp, $nonce) {
  try {
    db_insert('d2d_nonces')->fields(array(
        'nonce' => $nonce,
        'timestamp' => $timestamp,
    ))->execute();
  }
  catch (Exception $e) {
    return FALSE;
  }
  return TRUE;
}
