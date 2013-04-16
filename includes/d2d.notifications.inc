<?php
/**
 * @file
 * Notifications if instance is offline etc.,
 * a few functions to list notifications in a table, show details,
 * mark them as read, delete them etc.
 */

/**
 * Notify about instance being offline etc.
 */
function d2d_notify($exclude=array()) {
  d2d_instance_without_public_key();
  if (!in_array('offline', $exclude) && !d2d_is_online()) {
    drupal_set_message(t('D2D is currently in offline mode. In offline mode, no signed / encrypted queries will be sent and no queries are being answered. Click !link to change the settings.', array('!link' => l(t('here'), 'admin/d2d/settings/advanced'))), 'warning', FALSE);
  }
  if (!in_array('public_key', $exclude)) {
    $message = t('At least one instance in the database has no public key. Click !link to show an overview of all instances.', array('!link' => l(t('here'), 'admin/d2d')));
    if (d2d_instance_without_public_key()) {
      drupal_set_message($message, 'warning', FALSE);
    }
    elseif (isset($_SESSION['messages']['warning']) && (FALSE !== ($key = array_search($message, $_SESSION['messages']['warning'])))) {
      unset($_SESSION['messages']['warning'][$key]);
      if (empty($_SESSION['messages']['warning'])) {
        unset($_SESSION['messages']['warning']);
      }
    }
  }
  if (!in_array('notifications', $exclude)) {
    $n_unread = d2d_count_unread_notifications();
    if ($n_unread > 0) {
      if ($n_unread == 1) {
        drupal_set_message(t('You have @count unread notification. Click !link to read it.', array('@count' => $n_unread, '!link' => l(t('here'), 'admin/d2d/notifications'))), 'status', FALSE);
      }
      else {
        drupal_set_message(t('You have @count unread notifications. Click !link to read them.', array('@count' => $n_unread, '!link' => l(t('here'), 'admin/d2d/notifications'))), 'status', FALSE);
      }
    }
  }
}

/**
 * Count the number of unread notifications.
 * TODO: some of the notifications might not have a callback that allows to
 *       view them, hence they are counted as unread but also are not listed
 */
function d2d_count_unread_notifications() {
  $result = db_query('SELECT count(*) as unread_count FROM {d2d_notifications} WHERE seen=0');
  if ($row = $result->fetchAssoc()) {
    return $row['unread_count'];
  }
  else {
    return 0;
  }
}
/**
 * Returns TRUE iff there is an instance without a public key.
 *
 * TODO: improve the query
 */
function d2d_instance_without_public_key() {
  $result = db_query('SELECT EXISTS(SELECT 1 FROM (SELECT p.public_key as public_key FROM {d2d_instances} i LEFT OUTER JOIN {d2d_public_keys} p ON i.public_key_id = p.id) t WHERE t.public_key IS NULL) ex');
  if ($row = $result->fetchAssoc()) {
    return $row['ex'] != 0;
  }
  return FALSE;
}
/**
 * Implements hook_d2d_handle_notification().
 */
function d2d_d2d_handle_notification() {
  $types = array();
  $types['d2d_message'] = array(
    'arguments' => array('subject' => 'is_string', 'text' => 'is_string'),
    'callback_simple' => 'd2d_get_message_simple',
    'callback_detailed' => 'd2d_get_message_detailed',
    'name' => t('mail'),
  );
  $types['d2d_friendship'] = array(
    'arguments' => array('type' => 'is_string', 'description' => 'is_string'),
    'callback_simple' => 'd2d_get_friendship_simple',
    'callback_detailed' => 'd2d_get_friendship_detailed',
    'name' => t('friendship'),
  );
  return $types;
}

/**
 * Callback for getting a short description of a general message.
 */
function d2d_get_message_simple($arguments, $attributes) {
  return $arguments['subject'];
}

/**
 * Callback for getting a detailed description of a message.
 */
function d2d_get_message_detailed($arguments, $attributes) {
  $build = array();
  $build['notifications'] = array(
    '#title' => t('Notification') . ' #' . $attributes['id'],
    '#type' => 'fieldset',
    '#description' => $arguments['subject'] . ' - ' . $arguments['text'],
  );
  return $build;
}

/**
 * Insert a notification into the database.
 */
function d2d_insert_notification($type, $d2d_id, $content) {
  db_insert('d2d_notifications')->fields(array('type', 'd2d_id', 'content', 'timestamp'))
    ->values(array('type' => $type, 'd2d_id' => $d2d_id, 'content' => $content, 'timestamp' => d2d_get_time()))
    ->execute();
}
/**
 * Builts a table with notifications of type specified in $types
 *
 * If $types is an array, the type has to be contained in the array, otherwise
 * the table contains notifications of any type.
 */
function d2d_built_notification_table($types) {
  $build = array();
  $build['notifications'] = array(
    '#title' => t('Notifications'),
    '#type' => 'fieldset',
  );
  $hooks = module_invoke_all('d2d_handle_notification');
  $rows = array();
  if (is_array($types)) {
    if (empty($types)) {
      $result = array();
    }
    else {
      $result = db_query('SELECT id, type, d2d_id, content, timestamp, seen FROM {d2d_notifications} WHERE type IN (:types) ORDER BY id DESC', array(':types' => $types));
    }
  }
  else {
    $result = db_query('SELECT id, type, d2d_id, content, timestamp, seen FROM {d2d_notifications} ORDER BY id DESC');
  }
  foreach ($result as $record) {
    if (!array_key_exists($record->type, $hooks)) {
      continue;
    }
    $hook = $hooks[$record->type];
    $argument_checks = (array_key_exists('arguments', $hook) ? $hook['arguments'] : array());
    $arguments = d2d_explode($record->content);
    if (!d2d_check_array($arguments, $argument_checks)) {
      continue;
    }
    $attributes = array(
      'id' => $record->id,
      'd2d_id' => $record->d2d_id,
      'timestampe' => $record->timestamp,
    );
    $type = $hook['name'];
    $message = call_user_func($hook['callback_simple'], $arguments, $attributes);
    if ($record->seen) {
      $read_text = t('mark as unread');
      $read_link = 'admin/d2d/notifications/' . $record->id . '/mark_unread';
    }
    else {
      $read_text = t('mark as read');
      $read_link = 'admin/d2d/notifications/' . $record->id . '/mark_read';
    }
    $row = array(
      $record->id,
      $type,
      $message,
      d2d_get_absolute_time($record->timestamp),
      l(t('details'), 'admin/d2d/notifications/' . $record->id . '/details'),
      l($read_text, $read_link),
      l(t('delete'), 'admin/d2d/notifications/' . $record->id . '/delete'),
    );
    if (!$record->seen) {
      foreach ($row as &$cell) {
        $cell = '<b>' . $cell . '</b>';
      }
    }
    $rows['id' . $record->id] = $row;
  }
  $table = array(
    '#theme' => 'table',
    '#header' => array('ID', 'Type', 'Message', 'Time', array('data' => t('Operations'), 'colspan' => 3)),
    '#rows' => $rows,
    '#empty' => t('No notifications found.'),
  );
  return $table;
}

/**
 * Shows the notifications tab.
 */
function d2d_show_notifications() {
  d2d_notify(array('notifications'));
  $build = array();
  $build['notifications'] = array(
    '#title' => t('Notifications'),
    '#type' => 'fieldset',
  );
  //$build['notifications'][] = d2d_built_notification_table(array('d2d_message'));
  $build['notifications'][] = d2d_built_notification_table(NULL); 
  return $build;
}

/**
 * Shows details for the notification with id $id.
 */
function d2d_show_notification_details($id) {
  $id_not_found = FALSE;
  if (!d2d_check_convert_int($id)) {
    $id_not_found = TRUE;
  }
  if (!$id_not_found) {
    $record = db_query('SELECT id, type, d2d_id, content, timestamp, seen FROM {d2d_notifications} WHERE id=:id', array(':id' => $id));
    if ($row = $record->fetchObject()) {
      $content = $row->content;
      $attributes = array(
        'id' => $row->id,
        'd2d_id' => $row->d2d_id,
        'timestampe' => $row->timestamp,
      );
      $seen = $row->seen;
      $type = $row->type;
    }
    else {
      $id_not_found = TRUE;
    }
  }
  if ($id_not_found) {
    drupal_set_message(t('Notification not found'), 'error');
    drupal_goto('admin/d2d/notifications');
  }
  if (!$seen) {
    db_update('d2d_notifications')
      ->fields(array(
        'seen' => 1,
      ))
      ->condition('id', $id)
      ->execute();
  }
  $hooks = module_invoke_all('d2d_handle_notification');  
  $hook = $hooks[$type];
  $argument_checks = (array_key_exists('arguments', $hook) ? $hook['arguments'] : array());
  $arguments = d2d_explode($content);
  if (!d2d_check_array($arguments, $argument_checks)) {
    drupal_set_message('Notification erroneous', 'error');
    drupal_goto('admin/d2d/notifications');
  }
  $build = call_user_func($hook['callback_detailed'], $arguments, $attributes);
  return $build;
}

/**
 * Deletes a notification and redirects to the overview of notifications.
 */
function d2d_notification_delete_submit($form, &$form_state, $id) {
  if (d2d_check_convert_int($id)) {
    db_delete('d2d_notifications')
      ->condition('id', $id)
      ->execute();
  }
  drupal_goto('admin/d2d/notifications');
}

/**
 * Marks a notification as read and redirects to the overview of notifications.
 */
function d2d_notification_mark_read_submit($form, &$form_state, $id) {
  if (d2d_check_convert_int($id)) {
    $num_updated = db_update('d2d_notifications')
      ->fields(array(
        'seen' => 1,
      ))
      ->condition('id', $id)
      ->execute();
  }
  drupal_goto('admin/d2d/notifications');
}

/**
 * Marks a notification as unread and redirects to the overview of notifications.
 */
function d2d_notification_mark_unread_submit($form, &$form_state, $id) {
  if (d2d_check_convert_int($id)) {
    $num_updated = db_update('d2d_notifications')
      ->fields(array(
        'seen' => 0,
      ))
      ->condition('id', $id)
      ->execute();
  }
  drupal_goto('admin/d2d/notifications');
}
