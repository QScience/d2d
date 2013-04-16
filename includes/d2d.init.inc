<?php
/**
 * @file
 * Functions for adjusting the settings of D2D.
 */

function d2d_show_init() {
  $build['init'] = array(
    '#title' => t('Welcome to D2D'),
    '#type' => 'fieldset',
  );
  $build['init'][] = drupal_get_form('d2d_form_init');
//  d2d_notify();
  return $build;
}
function d2d_form_init() {
  $form = array();

  $form['introduction'] = array(
      '#markup' => t('Before using D2D, please provide a @length characters long D2D identifier. This identifier should be unique among all installations of D2D. It is recommended to generate that identifier randomly (e.g. by using the button below). If you installed D2D before, you can reuse the identifier of your old installation.', array('@length' => D2D_INSTANCE_IDENTIFIER_LENGTH)),
  );
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('A short name describing your instance.'),
    '#default_value' => 'My D2D',
    '#size' => D2D_INSTANCE_NAME_MAX_LENGTH,
    '#maxlength' => D2D_INSTANCE_NAME_MAX_LENGTH,
    '#required' => TRUE,
  );
  $form['id'] = array(
    '#type' => 'textfield',
    '#title' => t('D2D Identifier'),
    '#description' =>
      t(
        'Globally unique identifier consisting of exactly @length hexadecimal characters (A-F, 0-9).<br/>' . 
        'Note: once you have saved the global identifier, it cannot be changed anymore.',
        array('@length' => D2D_INSTANCE_IDENTIFIER_LENGTH)
      ),
    '#default_value' => '',
    '#size' => D2D_INSTANCE_IDENTIFIER_LENGTH,
    '#maxlength' => D2D_INSTANCE_IDENTIFIER_LENGTH,
    '#required' => FALSE, // length is checked anyway, no error if 'generate'-button is pressed
  );
  $form['submit'] = array(
    '#type' => 'submit', 
    '#value' => t('Save and continue'),
  );
  $form['generate'] = array(
    '#type' => 'submit', 
    '#value' => t('Generate random identifier'),
    '#validate' => array('d2d_form_init_generate_validate'),
    '#submit' => array(),
  );
  return $form;
}
function d2d_form_init_validate($form, &$form_state) {
  $id = $form_state['values']['id'];
  if (!d2d_check_d2d_id_length($id)) {
    form_set_error('id', t('Identifier must constist of exactly @length characters.', array('@length' => D2D_INSTANCE_IDENTIFIER_LENGTH)));
  }
  if (!d2d_is_hex_string($id)) {
    form_set_error('id', t('Identifier must consists only of hexadecimal characters (A-F, 0-9).'));
  }
}
function d2d_form_init_submit($form, &$form_state) {
  $id = $form_state['values']['id'];
  // add own instance to database
  $my_id = db_insert('d2d_instances')->fields(array(
      'name' => $form_state['values']['name'],
      'url' => $GLOBALS['base_url'] . '/xmlrpc.php',
      'd2d_id' => $id,
      'description' => 'this instance.',
      'time_inserted' => d2d_get_time(),
      'public_key_id' => NULL,
    ))->execute();
  variable_set('d2d_my_id', $my_id);
  menu_rebuild();
  drupal_set_message(t('Settings have been saved.'));
}
function d2d_form_init_generate_validate($form, &$form_state) {
  $id = d2d_random_d2d_id();
  $form_state['complete form']['id']['#value'] = $id;
  form_set_error(''); // TODO: this is basically a hack, better ways to do this?
  drupal_set_message(t('Click \'Save & continue\' to proceed with this random identifier.'));
}
