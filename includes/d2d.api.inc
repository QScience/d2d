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

function d2d_api_delete_public_key($public_key_id) {
	$num_updated = db_delete('d2d_public_keys')
      ->condition('id', $public_key_id)
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

