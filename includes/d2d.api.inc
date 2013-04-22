<?php
/**
 * @file
 * Low levels functions used by the forms submit handlers of the D2D module.
 */

function d2d_api_update_friendship($friend_id, $friendship_state) {
	db_update('d2d_instances')
  	->fields(array(
    		'friendship_state' => $friendship_state,
			))
      ->condition('id', $friend_id)
      ->execute();
        
	// call hook  
	module_invoke_all('d2d_friendship_update', $friend_id, $friendship_state);       
}

function d2d_api_remove_instance($instance_id) {
	
	$instance = d2d_get_instance2($id);
	
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

function d2d_api_select_public_key($instance_id, $public_key_id) {
	$num_updated = db_update('d2d_instances')
		->fields(array(
        'public_key_id' => $public_key_id,
    	))
			->condition('id', $instance_id)
    	->execute();
    	
	return $num_updated;
}

function d2d_api_unselect_public_key($instance_id) {
	$num_updated = db_update('d2d_instances')
		->fields(array(
    		'public_key_id' => NULL,
    	))
    	->condition('id', $instance_id)
      ->execute();
      
	return $num_updated;	
}

function d2d_api_insert_public_key($instance_id, $public_key) {
	$id_pk = db_insert('d2d_public_keys')->fields(array(
  	'instance_id' => $instance_id,
    'public_key' => $public_key,
  ))->execute();
 
  return $id_pk;
}

function d2d_api_update_public_key($id, $update) {
	$num_updated = db_update('d2d_public_keys')
  	->fields($update)
  	->condition('id', $id)
  	->execute();
 
  return $num_updated;
}


function d2d_api_delete_public_key($id) {
	$num_deleted = db_delete('d2d_public_keys')
		->condition('id', $id)
	 	->execute();
 
  return $num_deleted;
}



function d2d_api_update_instance($instance_id, $fields) {
	$num_updated = db_update('d2d_instances')
  	->fields($fields)
    ->condition('id', $instance_id)
    ->execute();
    
	return $num_updated;
}

function d2d_api_update_group_memberships($instance_id, $group_ids = array()) {
	db_delete('d2d_group_memberships')
		->condition('instance_id', $instance_id)->execute();
  
	if (!empty($group_ids)) {	
		d2d_api_add_group_memberships($instance_id, $group_ids);
	}
	else {
		// call hook  
		module_invoke_all('d2d_group_membership', $instance_id, $group_ids); 
	}
}

function d2d_api_add_group_memberships($instance_id, $group_ids = array()) {
	if (!empty($group_ids)) {	
		$query = db_insert('d2d_group_memberships')->fields(array('group_id', 'instance_id'));
	  foreach ($group_ids as $gid) {
	  	$query->values(array('group_id' => $gid, 'instance_id' => $instance_id));
		}
	  $query->execute();
	
	  // call hook  
		module_invoke_all('d2d_group_membership', $instance_id, $group_ids); 
	}
}

function d2d_api_get_public_keys($d2d_id = NULL) {
 
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

function d2d_api_get_public_keys_array($d2d_id = NULL) {
	$keys = d2d_api_get_public_keys($d2d_id);
  $out = array();
	if (empty($keys)) return $out;

  foreach ($keys as $k) {
  	$out[] = get_object_vars($k);
  }
  return $out;
}

function d2d_api_add_instance($instance) {
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

function d2d_api_instance_exist($d2d_id) {

	$query = db_select('d2d_instances', 'i')
		->condition('i.d2d_id', $d2d_id);
  
	$res = $query->execute()->FetchObject();	
  return empty($res);
}

function d2d_api_get_public_key_id($d2d_id, $public_key) {
	$keys = d2d_api_get_public_keys($d2d_id);
	if (empty($keys)) return FALSE;
	foreach ($keys as $k) {
		if (d2d_public_key_eql($k->public_key, $public_key)) {
			return $k->id;
		}
	}
  return FALSE;
}


function d2d_api_public_key_exist($d2d_id, $public_key) {
	 return d2d_api_get_public_key_id($d2d_id, $public_key) !== FALSE;
}

function d2d_api_get_public_key_by_id($id) {
 
  $query = db_select('d2d_public_keys', 'k');

	$query->join('d2d_instances', 'i', 'i.id = k.instance_id');
	$query
	 	->fields('i', array('d2d_id'))
		->fields('k', array('id', 'public_key'))
 		->condition('k.id', $id);
            
  return $query->execute()->FetchObject();
}

