<?php
/**
 * @file
 * Low levels functions used by the forms submit handlers of the D2D module.
 */

function d2d_api_friendship_update($friend_id, $friendship_state) {
	db_update('d2d_instances')
  	->fields(array(
    		'friendship_state' => $friendship_state,
			))
      ->condition('id', $friend_id)
      ->execute();
        
	// call hook  
	module_invoke_all('d2d_friendship_update', $friend_id, $friendship_state);       
}

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

function d2d_api_public_key_select($instance_id, $public_key_id) {
	$num_updated = db_update('d2d_instances')
		->fields(array(
        'public_key_id' => $public_key_id,
    	))
			->condition('id', $instance_id)
    	->execute();
    	
	return $num_updated;
}

function d2d_api_public_key_unselect($instance_id) {
	$num_updated = db_update('d2d_instances')
		->fields(array(
    		'public_key_id' => NULL,
    	))
    	->condition('id', $instance_id)
      ->execute();
      
	return $num_updated;	
}

function d2d_api_public_key_set($d2d_id, $public_key) {
	$instance_id = d2d_convert_d2d_id_to_id($d2d_id);
	$id_pk = db_insert('d2d_public_keys')->fields(array(
  	'instance_id' => $instance_id,
    'public_key' => $public_key,
  ))->execute();
 
  return $id_pk;
}

function d2d_api_public_key_update($id, $update) {
	$num_updated = db_update('d2d_public_keys')
  	->fields($update)
  	->condition('id', $id)
  	->execute();
 
  return $num_updated;
}


function d2d_api_public_key_delete($id) {
	$num_deleted = db_delete('d2d_public_keys')
		->condition('id', $id)
	 	->execute();
 
  return $num_deleted;
}



function d2d_api_instance_update($instance_id, $fields) {
	$num_updated = db_update('d2d_instances')
  	->fields($fields)
    ->condition('id', $instance_id)
    ->execute();
    
	return $num_updated;
}

function d2d_api_group_membership_update($instance_id, $group_ids = array()) {
	db_delete('d2d_group_memberships')
		->condition('instance_id', $instance_id)->execute();
  
	if (!empty($group_ids)) {	
		d2d_api_group_memberships_add($instance_id, $group_ids);
	}
	else {
		// call hook  
		module_invoke_all('d2d_group_membership', $instance_id, $group_ids); 
	}
}

function d2d_api_group_membership_add($instance_id, $group_ids = array()) {
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

function d2d_api_public_key_get_array($d2d_id = NULL) {
	$keys = d2d_api_public_key_get($d2d_id);
  $out = array();
	if (empty($keys)) return $out;

  foreach ($keys as $k) {
  	$out[] = get_object_vars($k);
  }
  return $out;
}

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

function d2d_api_instance_exist($d2d_id) {

	$query = db_select('d2d_instances', 'i')
		->condition('i.d2d_id', $d2d_id);
  
	$res = $query->execute()->FetchObject();	
  return empty($res);
}

function d2d_api_public_key_return_id($d2d_id, $public_key) {
	$keys = d2d_api_public_key_get($d2d_id);
	if (empty($keys)) return FALSE;
	foreach ($keys as $k) {
		if (d2d_public_key_eql($k->public_key, $public_key)) {
			return $k->id;
		}
	}
  return FALSE;
}


function d2d_api_public_key_exist($d2d_id, $public_key) {
	 return d2d_api_public_key_return_id($d2d_id, $public_key) !== FALSE;
}

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

  if ($reset || empty($key)) {
  	$my_d2d_id = d2d_api_own_d2d_id_get();
  	$key = d2d_api_public_key_get($my_d2d_id);
  	$key = empty($key) ? FALSE: $key[0]->public_key; 
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

function d2d_api_own_public_key_del() {
	
	$num_deleted = db_delete('d2d_public_keys')
		->condition('id_instance', $id)
	 	->execute();
	 	
	return variable_del('d2d_public_key');
}


function d2d_api_own_private_key_get() {
	return variable_get('d2d_private_key', FALSE);
}

function d2d_api_own_private_key_set($private_key) {
	return variable_set('d2d_private_key', $private_key);
}

function d2d_api_own_private_key_del() {
	return variable_del('d2d_private_key');
}

function d2d_api_own_d2d_id_get() {
	$my_d2d_id = &drupal_static(__FUNCTION__);
	if (empty($my_d2d_id)) {
		$my_d2d_id = variable_get('d2d_my_id');
	}
	return $my_d2d_id;
}

function d2d_api_own_d2d_id_set($d2d_id) {	
	return variable_set('d2d_my_id', $d2d_id);
}

function d2d_api_own_d2d_id_del($d2d_id) {	
	return variable_del('d2d_my_id');
}

function d2d_api_own_instance_get() {
  $id = d2d_api_own_instance_id_get();
  return d2d_api_instance_get_by_id($id);
}

function d2d_api_own_instance_id_get() {
	$id = &drupal_static(__FUNCTION__);
	if (empty($id)) {
		$id = variable_get('d2d_my_instance_id');
	}
	return $id;
}

function d2d_api_own_instance_id_set($id) {	
	return variable_set('d2d_my_instance_id', $id);
}

function d2d_api_own_instance_id_del($id) {	
	return variable_del('d2d_my_instance_id');
}

function d2d_api_instance_get_by_id($id = NULL, $reset = FALSE) {
	
	$instances = &drupal_static(__FUNCTION__);
		
	if (!empty($d2d_id)) {
    $query = $query->condition('i.d2d_id', $d2d_id);
  }
	
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

function d2d_api_friend_get_by_url($url, $reset = FALSE) {
	$friends = d2d_api_friend_get($reset);
  foreach ($friends as $friend) {
    if ($friend['url'] === $url) {
      return $friend;
    }
  }
  return FALSE;
}

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