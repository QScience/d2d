<?php
/**
 * @file
 * Database related functions.
 */

// Update state of friendship.
function d2d_api_friendship_update($friend_id, $friendship_state) {
  db_update('d2d_instances')
    ->fields(array('friendship_state' => $friendship_state))
    ->condition('id', $friend_id)
    ->execute();
  // call hook  
  module_invoke_all('d2d_friendship_update', $friend_id, $friendship_state);       
}

// Remove instance and all associated entries from the database.
function d2d_api_instance_remove($instance_id) {
  $instance = d2d_api_instance_get_by_id($instance_id);
  // remove group memberships
  db_delete('d2d_group_memberships')
    ->condition('instance_id', $instance_id)
    ->execute();
  // remove public keys associated with this instance
  db_delete('d2d_public_keys')
    ->condition('instance_id', $instance_id)
    ->execute();
  // remove incoming requests
  db_delete('d2d_incoming_requests')
    ->condition('sender_d2d_id', $instance['d2d_id'])
    ->execute();
  // remove outgoing requests
  db_delete('d2d_outgoing_requests')
    ->condition('receiver_id', $instance_id)
    ->execute();
  // remove instance
  db_delete('d2d_instances')
    ->condition('id', $instance_id)
    ->execute();
  // call hook  
  module_invoke_all('d2d_instance_removed', $instance); 
}

// Selects public key of an instance.
function d2d_api_public_key_select($instance_id, $public_key_id) {
  $num_updated = db_update('d2d_instances')
    ->fields(array('public_key_id' => $public_key_id))
    ->condition('id', $instance_id)
    ->execute();
  return $num_updated;
}

// Unselects public key of an instance.
function d2d_api_public_key_unselect($instance_id) {
  return d2d_api_public_key_select($instance_id, NULL);
}

// TODO: $d2d_id -> $id, all function in the D2D-module have the id
//  and not just the $d2d_id available 
function d2d_api_public_key_set($d2d_id, $public_key) {
  $instance_id = d2d_convert_d2d_id_to_id($d2d_id);
  $id_pk = db_insert('d2d_public_keys')->fields(array(
    'instance_id' => $instance_id,
    'public_key' => $public_key,
  ))->execute();
  return $id_pk;
}

// TODO: used by d2d_patterns only
function d2d_api_public_key_update($id, $update) {
  $num_updated = db_update('d2d_public_keys')
    ->fields($update)
    ->condition('id', $id)
    ->execute();
 
  return $num_updated;
}

// Removes public key from the database.
function d2d_api_public_key_delete($id) {
  $num_deleted = db_delete('d2d_public_keys')
    ->condition('id', $id)
    ->execute();
  return $num_deleted;
}

// Updates fields of an instance.
function d2d_api_instance_update($instance_id, $fields) {
  $num_updated = db_update('d2d_instances')
    ->fields($fields)
    ->condition('id', $instance_id)
    ->execute();
  return $num_updated;
}

// TODO: better use is_null than empty
// 
function d2d_api_public_key_get($d2d_id = NULL) {
 
  $query = db_select('d2d_public_keys', 'k');

  $query->join('d2d_instances', 'i', 'i.id = k.instance_id');
  $query
     ->fields('i', array('d2d_id'))
    ->fields('k', array('id', 'public_key'));
 
            
  if (!empty($d2d_id)) {
    $query = $query->condition('i.d2d_id', $d2d_id);
  }

   $keys = $query->execute()->FetchAll();

  return $keys;
}

// TODO: used by d2d_patterns only
function d2d_api_public_key_get_array($d2d_id = NULL) {
  $keys = d2d_api_public_key_get($d2d_id);
  $out = array();
  if (empty($keys)) return $out; // TODO: do we need this condition?

  foreach ($keys as $k) {
    $out[] = get_object_vars($k);
  }
  return $out;
}

// TODO: this function is not used at all
//       why not return $id?
function d2d_api_instance_add($instance) {
  $id = db_insert('d2d_instances')->fields(array(
      'd2d_id' => $instance['d2d_id'],
      'name' => $instance['name'],
      'url' => $instance['address'],
      'description' => $instance['description'],
      'time_inserted' => d2d_get_time(),
      'public_key_id' => NULL,
   ))->execute();
    
  d2d_set_default_group_memberships($id);
}

// TODO: this function is not used at all
//       probably returns the opposite of what it should do anyway
function d2d_api_instance_exist($d2d_id) {

  $query = db_select('d2d_instances', 'i')
    ->condition('i.d2d_id', $d2d_id);
  
  $res = $query->execute()->FetchObject();  
  return empty($res);
}

function d2d_api_public_key_return_id($d2d_id, $public_key) {
  $keys = d2d_api_public_key_get($d2d_id);
  if (empty($keys)) return FALSE; // TODO: is this condition needed?
  foreach ($keys as $k) {
    if (d2d_public_key_eql($k->public_key, $public_key)) {
      return $k->id;
    }
  }
  return FALSE;
}

// used by d2d_patterns only
function d2d_api_public_key_exist($d2d_id, $public_key) {
   return d2d_api_public_key_return_id($d2d_id, $public_key) !== FALSE;
}

// used by patterns module only
function d2d_api_public_key_get_by_id($id) {
 
  $query = db_select('d2d_public_keys', 'k');

  $query->join('d2d_instances', 'i', 'i.id = k.instance_id');
  $query
     ->fields('i', array('d2d_id'))
    ->fields('k', array('id', 'public_key'))
     ->condition('k.id', $id);
            
  return $query->execute()->FetchObject();
}


function d2d_api_own_public_key_get($reset = FALSE) {
  
  $key = &drupal_static(__FUNCTION__);

  if ($reset || empty($key)) { // TODO: is_null instead of empty
    $my_d2d_id = d2d_api_own_d2d_id_get();
    $key = d2d_api_public_key_get($my_d2d_id);
    $key = empty($key) ? FALSE: $key[0]->public_key; // TODO: $key === FALSE instead of empty($key)
  }
  return $key;
}

function d2d_api_own_public_key_set($public_key) {
  $my_d2d_id = d2d_api_own_d2d_id_get();
  $id = d2d_api_own_instance_id_get();
  $id_pk = d2d_api_public_key_set($my_d2d_id, $public_key);
  d2d_api_instance_update($id, array(
      'public_key_id' => $id_pk,
  )); 

  db_delete('d2d_public_keys')
    ->condition('id', $id_pk, '<>')
    ->condition('instance_id', $id)
    ->execute();
  
  return $id_pk;
}

// TODO: used by d2d_patterns only, can possibly break things
//   variable_del has no return value
function d2d_api_own_public_key_del() {
  
  $num_deleted = db_delete('d2d_public_keys')
    ->condition('id_instance', $id)
     ->execute();
     
  return variable_del('d2d_public_key');
}


function d2d_api_own_private_key_get() {
  return variable_get('d2d_private_key', FALSE);
}


// TODO: variable_set has no return value
function d2d_api_own_private_key_set($private_key) {
  return variable_set('d2d_private_key', $private_key);
}

// TODO: variable_del has no return value
function d2d_api_own_private_key_del() {
  return variable_del('d2d_private_key');
}

// TODO: better use is_null than empty
function d2d_api_own_d2d_id_get() {
  $my_d2d_id = &drupal_static(__FUNCTION__);
  if (empty($my_d2d_id)) {
    $my_d2d_id = variable_get('d2d_my_id');
  }
  return $my_d2d_id;
}

// TODO: variable_set has no return value
function d2d_api_own_d2d_id_set($d2d_id) {  
  return variable_set('d2d_my_id', $d2d_id);
}

// TODO: variable_del has no return value
//   function not used at all
function d2d_api_own_d2d_id_del($d2d_id) {  
  return variable_del('d2d_my_id');
}

function d2d_api_own_instance_get() {
  $id = d2d_api_own_instance_id_get();
  return d2d_api_instance_get_by_id($id);
}

// TODO: better use is_null than empty
function d2d_api_own_instance_id_get() {
  $id = &drupal_static(__FUNCTION__);
  if (empty($id)) {
    $id = variable_get('d2d_my_instance_id');
  }
  return $id;
}

// TODO: variable_set has no return value
function d2d_api_own_instance_id_set($id) {  
  return variable_set('d2d_my_instance_id', $id);
}

// TODO: variable_del has no return value
//  function not used at all
function d2d_api_own_instance_id_del($id) {  
  return variable_del('d2d_my_instance_id');
}

// TODO: this is basically the same function as d2d_api_instance_get,
//  merge them
function d2d_api_instance_get_by_id($id = NULL, $reset = FALSE) {
  
  $instances = &drupal_static(__FUNCTION__);
    
//  if (!empty($d2d_id)) {
//    $query = $query->condition('i.d2d_id', $d2d_id);
//  }
  
  if ($reset || empty($instances) || empty($instances[$id])) {
    $query = db_select('d2d_instances', 'i');
    $query->leftJoin('d2d_public_keys', 'k', 'i.public_key_id = k.id');
    $query
      ->fields('k', array('public_key'))
       ->fields('i')
      ->orderBy('i.id', 'asc');
      
     // Return only one instance
    if (!empty($id)) {
      $query = $query->condition('i.id', $id);
      $result = $query->execute();
      $instance = $result->fetchAssoc();
      if (empty($instance)) return FALSE;
      
      _d2d_decorate_instance_array($instance);
      return $instance;
    }

    // Return multiple instance in associative array
    $result = $query->execute();
    $instances = array();
    foreach ($result as $record) {
      $instances[$record->id] = array(
        'id' => $record->id,
        'd2d_id' => $record->d2d_id,
        'name' => $record->name,
        'url' => $record->url,
        'description' => $record->description,
        'time_inserted' => $record->time_inserted,
        'last_alive' => $record->last_alive,
        'public_key' => $record->public_key,
        'friendship_state' => $record->friendship_state,
      );
      _d2d_decorate_instance_array($instances[$record->id]);
    }
  }
  
  return $instances;
}

// TODO: remove caching, it breaks things if $d2d_id is not NULL
//       change ambiguous usage of empty(...), better use is_null
function d2d_api_instance_get($d2d_id = NULL, $reset = FALSE) {
  $instances = &drupal_static(__FUNCTION__);
    
  if ($reset || empty($instances) || empty($instances[$d2d_id])) {
    $query = db_select('d2d_instances', 'i');
    $query->leftJoin('d2d_public_keys', 'k', 'i.public_key_id = k.id');
    $query
      ->fields('k', array('public_key'))
       ->fields('i');
    
     // Return only one instance
    if (!empty($d2d_id)) {
      $query = $query->condition('i.d2d_id', $d2d_id);
      $result = $query->execute();
      $instance = $result->fetchAssoc();
      
      if (empty($instance)) return FALSE;    
      
      _d2d_decorate_instance_array($instance);
      return $instance;
    }

    // Return multiple instance in associative array
    $result = $query->execute();
    $instances = array();
    foreach ($result as $record) {
      $instances[$record->d2d_id] = array(
        'id' => $record->id,
        'd2d_id' => $record->d2d_id,
        'name' => $record->name,
        'url' => $record->url,
        'description' => $record->description,
        'time_inserted' => $record->time_inserted,
        'last_alive' => $record->last_alive,
        'public_key' => $record->public_key,
        'friendship_state' => $record->friendship_state,
      );
      _d2d_decorate_instance_array($instances[$record->d2d_id]);
    }
  }
  
  return $instances;
}

// TODO: remove caching
function d2d_api_friend_get_by_url($url, $reset = FALSE) {
  $friends = d2d_api_friend_get($reset);
  foreach ($friends as $friend) {
    if ($friend['url'] === $url) {
      return $friend;
    }
  }
  return FALSE;
}

// TODO: this is basically the same function as d2d_api_friend_get_by_id,
//  merge them
function d2d_api_friend_get($reset = FALSE) {
  
  $friends = &drupal_static(__FUNCTION__);
  
  if ($reset || empty($friends)) {
    $query = db_select('d2d_instances', 'i');
    $query->leftJoin('d2d_public_keys', 'k', 'i.public_key_id = k.id');
    $query = $query  
                ->fields('i')
                ->fields('k', array('public_key'))
                ->condition('i.friendship_state', 3);
                 
    $result = $query->execute();
    
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
  }

  return $friends;
}


// TODO: remove caching (it does not work if $friends is empty anyway)
function d2d_api_friend_get_by_id($reset = FALSE) {
  
  $friends = &drupal_static(__FUNCTION__);
  
  if ($reset || empty($friends)) {
    $query = db_select('d2d_instances', 'i');
    $query->leftJoin('d2d_public_keys', 'k', 'i.public_key_id = k.id');
    $query = $query  
                ->fields('i')
                ->fields('k', array('public_key'))
                ->condition('i.friendship_state', 3);
                 
    $result = $query->execute();
    
    $friends = array();
    foreach ($result as $record) {
      $friends[$record->id] = array(
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
  }

  return $friends;
}

/////////////////////////////////////////////////////////////////

/**
 * Returns a group by its id resp. all groups.
 */
function d2d_api_group_get($group_id = NULL) {
  $query = db_select('d2d_groups', 'g')->fields('g');
  if (is_null($group_id)) {
    $result = $query->execute();
    $groups = array();
    foreach ($result as $record) {
      $groups[$record->id] = array(
        'id' => $record->id,
        'name' => $record->name,
        'description' => $record->description,
        'auto_member' => $record->auto_member,
      );
    }
    return $groups;
  }
  else {
    $query = $query->condition('g.id', $group_id);
    $result = $query->execute();
    $group = $result->fetchAssoc();
    return $group ? $group : NULL;
  }
}

/**
 * Returns group by name.
 */
function d2d_api_group_get_by_name($group_name) {
  $query = db_select('d2d_groups', 'g')->fields('g');
  $query = $query->condition('g.name', $group_name);
  $result = $query->execute();
  $group = $result->fetchAssoc();
  return $group ? $group : NULL;
}

/**
 * Removes specified group.
 */
function d2d_api_group_delete($group_ids) {
  if (empty($group_ids)) {
    return 0;
  }
  else {
    return db_delete('d2d_groups')
      ->condition('id', $group_ids)
      ->execute();
  }
}

/**
 * Returns permissions.
 */
function d2d_api_permission_get($look_up_table = FALSE) {
  $query = db_select('d2d_permissions', 'p')->fields('p');
  $result = $query->execute();
  $permissions = array();
  foreach ($result as $record) {
    $permissions[] = array(
      'group_id' => $record->group_id,
      'method' => $record->method,
    );
  }
  if (!$look_up_table) {
    return $permissions;
  }
  else {
    $permission_table = array();
    foreach ($permissions as $permission) {
      $permission_table[$permission['group_id']][$permission['method']] = 1;
    }
    return $permission_table;
  }
}

/**
 * Clears permissions.
 */
function d2d_api_permission_clear() {
  return db_delete('d2d_permissions')->execute();
}

/**
 * Adds permissions.
 */
function d2d_api_permission_add($permissions) {
  $query = db_insert('d2d_permissions')->fields(array('group_id', 'method'));
  foreach ($permissions as $permission) {
    $query->values($permission);
  }
  $query->execute();
}

/**
 * Removes all permissions for groups specified as argument.
 */
function d2d_api_permission_delete($group_ids) {
  if (empty($group_ids)) {
    return 0;
  }
  else {
    return db_delete('d2d_permissions')
      ->condition('group_id', $group_ids)
      ->execute();
  }
}

/**
 * Creates a new group.
 */
function d2d_api_group_add($name, $description, $auto_member) {
  try {
    $id = db_insert('d2d_groups')->fields(
      array(
        'name' => $name,
        'description' => $description,
        'auto_member' => $auto_member ? 1 : 0,
      ))->execute();
    return $id;
  }
  catch (Exception $e) {
    return NULL;
  }
}

/**
 * Updates attributes of a group.
 */
function d2d_api_group_update($id, $name, $description, $auto_member) {
  return db_update('d2d_groups')
    ->fields(array(
      'name' => $name,
      'description' => $description,
      'auto_member' => $auto_member ? 1 : 0,
    ))
    ->condition('id', $id)
    ->execute();
}

/**
 * Returns all groups with a flag indicating group membership of given instance.
 */
function d2d_api_group_membership_get($instance_id) {
  $query = db_select('d2d_groups', 'g');
  $query->leftJoin('d2d_group_memberships', 'gm', '(g.id = gm.group_id AND gm.instance_id = :instance_id)', array(':instance_id' => $instance_id));
  $query = $query->fields('g')
                 ->fields('gm', array('group_id'));
  $result = $query->execute();
  $groups = array();
  foreach ($result as $record) {
    $groups[$record->id] = array(
      'id' => $record->id,
      'name' => $record->name,
      'description' => $record->description,
      'auto_member' => $record->auto_member,
      'is_member' => !is_null($record->group_id),
    );
  }
  return $groups;
}

/**
 * Removes all instances from groups specified as argument.
 */
function d2d_api_group_membership_delete($group_ids) {
  if (empty($group_ids)) {
    return 0;
  }
  else {
    return db_delete('d2d_group_memberships')
      ->condition('group_id', $group_ids)
      ->execute();
  }
}

/**
 * Checks if instance may call a mathod if it is a friend.
 */
function d2d_api_method_permission_exists($instance_id, $method) {
  $query = db_select('d2d_group_memberships', 'gm');
  $query = $query->condition('gm.instance_id', $instance_id);
  $query->join('d2d_permissions', 'p', '(gm.group_id = p.group_id AND p.method = :method)', array(':method' => $method));
  $query = $query->fields('p', array('method'));
  $result = $query->execute();
  return $result->fetchAssoc() ? TRUE : FALSE;
}

/**
 * Returns a list of methods an instance may call if it is a friend.
 */
function d2d_api_method_get($instance_id, $look_up_table=FALSE) {
  $query = db_select('d2d_group_memberships', 'gm');
  $query = $query->condition('gm.instance_id', $instance_id);
  $query->join('d2d_permissions', 'p', 'gm.group_id = p.group_id');
  $query = $query->fields('p', array('method'));
  $query = $query->distinct();
  $result = $query->execute();
  if ($look_up_table) {
    $method_table = array();
    foreach ($result as $record) {
      $method_table[$record->method] = 1;
    }
    return $method_table;
  }
  else {
    $methods = array();
    foreach ($result as $record) {
      $methods[] = $record->method;
    }
    return $methods;
  }
}

function d2d_api_group_membership_update($instance_id, $group_ids = array()) {
  db_delete('d2d_group_memberships')
    ->condition('instance_id', $instance_id)->execute();
  
  if (!empty($group_ids)) {  
    d2d_api_group_membership_add($instance_id, $group_ids);
  }
  else {
    // call hook  
    module_invoke_all('d2d_group_membership', $instance_id, $group_ids); 
  }
}

function d2d_api_group_membership_add($instance_id, $group_ids = array()) {
  if (!empty($group_ids)) {  
    $query = db_insert('d2d_group_memberships')->fields(array('group_id', 'instance_id'));
    foreach ($group_ids as $group_id) {
      $query->values(array('group_id' => $group_id, 'instance_id' => $instance_id));
    }
    $query->execute();
  
    // call hook  
    module_invoke_all('d2d_group_membership', $instance_id, $group_ids); 
  }
}

// notification
/**
 * Counts the number of unread notifications.
 * TODO: some of the notifications might not have a callback that allows to
 *       view them, hence they are counted as unread but also are not listed
 */
function d2d_api_notification_count_unread() {
  $query = db_select('d2d_notifications', 'n');
  $query = $query->condition('n.seen', 0);
  $query = $query->fields('n', array('id'));
  $result = $query->execute();
  return $result->rowCount();
}

/**
 * Inserts a notification into the database.
 */
function d2d_api_notification_insert($type, $d2d_id, $content) {
  db_insert('d2d_notifications')->fields(array('type', 'd2d_id', 'content', 'timestamp'))
    ->values(array('type' => $type, 'd2d_id' => $d2d_id, 'content' => $content, 'timestamp' => d2d_get_time()))
    ->execute();
}
/**
 * Marks a notification as read or unread.
 */
function d2d_api_notification_change_read($notification_id, $seen) {
  return db_update('d2d_notifications')
    ->fields(array(
      'seen' => $seen ? 1 : 0,
    ))
    ->condition('id', $notification_id)
    ->execute();
}

/**
 * Deletes a notification.
 */
function d2d_api_notification_delete($notification_id) {
  return db_delete('d2d_notifications')
    ->condition('id', $notification_id)
    ->execute();
}

function d2d_api_notification_get_by_type($types) {
  $query = db_select('d2d_notifications', 'n');
  if (!is_null($types)) {
    if (empty($types)) {
      return array();
    }
    $query = $query->condition('n.type', $types);
  }
  $query = $query->fields('n')
                 ->orderBy('n.id', 'DESC');
  $result = $query->execute();
  $notifications = array();
  foreach ($result as $record) {
    $notifications[] = array(
      'id' => $record->id,
      'type' => $record->type,
      'd2d_id' => $record->d2d_id,
      'content' => $record->content,
      'timestamp' => $record->timestamp,
      'seen' => $record->seen,
    );
  }
  return $notifications;
}
function d2d_api_notification_get($notification_id) {
  $query = db_select('d2d_notifications', 'n');
  $query = $query->condition('n.id', $notification_id);
  $query = $query->fields('n');
  $result = $query->execute();
  $notification = $result->fetchAssoc();
  return $notification ? $notification : NULL;
}
