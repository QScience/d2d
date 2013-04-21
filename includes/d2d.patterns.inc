<?php
/**
 * @file
 * Hook functions for Patterns integration
 */

function d2d_patterns($data = NULL){
	
	$tags['d2d_public_key'] = array(
    PATTERNS_INFO   => t('Add/Modify/Delete public keys'),
    PATTERNS_CREATE => array('taxonomy_form_vocabulary'),
    PATTERNS_MODIFY => array('taxonomy_form_vocabulary'),
    PATTERNS_DELETE => array('taxonomy_vocabulary_confirm_delete'),
    PATTERNS_FILES  => array('modules/taxonomy/taxonomy.admin.inc'),
    PATTERNS_EXPORT => array(
                          PATTERNS_EXPORT_ALL => 'taxonomy_patterns_export_all_vocs',
                          // Commented are more options for extraction
                          // This is not yet working
                          // 'Cheat (I am doing terms)' => 'taxonomy_patterns_export_all_terms',
                          //'Single Vocabulary' => array('callback' => 'taxonomy_patterns_export_single_vocabulary',
                          //                             'arguments' => array('1', '2', '3'),
                          //),
                        ),
  );
  
  
  $tags['d2d_instance'] = array(
    PATTERNS_INFO   => t('Create/Modify/Delete Taxonomy Vocabularies'),
    PATTERNS_CREATE => array('taxonomy_form_vocabulary'),
    PATTERNS_MODIFY => array('taxonomy_form_vocabulary'),
    PATTERNS_DELETE => array('taxonomy_vocabulary_confirm_delete'),
    PATTERNS_FILES  => array('modules/taxonomy/taxonomy.admin.inc'),
    PATTERNS_EXPORT => array(
                          PATTERNS_EXPORT_ALL => 'taxonomy_patterns_export_all_vocs',
                          // Commented are more options for extraction
                          // This is not yet working
                          // 'Cheat (I am doing terms)' => 'taxonomy_patterns_export_all_terms',
                          //'Single Vocabulary' => array('callback' => 'taxonomy_patterns_export_single_vocabulary',
                          //                             'arguments' => array('1', '2', '3'),
                          //),
                        ),
  );
  
  $tags['d2d_friendship'] = array(
    PATTERNS_INFO   => t('Create/Modify/Delete Taxonomy Vocabularies'),
    PATTERNS_CREATE => array('taxonomy_form_vocabulary'),
    PATTERNS_MODIFY => array('taxonomy_form_vocabulary'),
    PATTERNS_DELETE => array('taxonomy_vocabulary_confirm_delete'),
    PATTERNS_FILES  => array('modules/taxonomy/taxonomy.admin.inc'),
    PATTERNS_EXPORT => array(
                          PATTERNS_EXPORT_ALL => 'taxonomy_patterns_export_all_vocs',
                          // Commented are more options for extraction
                          // This is not yet working
                          // 'Cheat (I am doing terms)' => 'taxonomy_patterns_export_all_terms',
                          //'Single Vocabulary' => array('callback' => 'taxonomy_patterns_export_single_vocabulary',
                          //                             'arguments' => array('1', '2', '3'),
                          //),
                        ),
  );
  
  $tags['d2d_group'] = array(
    PATTERNS_INFO   => t('Create/Modify/Delete Taxonomy Vocabularies'),
    PATTERNS_CREATE => array('taxonomy_form_vocabulary'),
    PATTERNS_MODIFY => array('taxonomy_form_vocabulary'),
    PATTERNS_DELETE => array('taxonomy_vocabulary_confirm_delete'),
    PATTERNS_FILES  => array('modules/taxonomy/taxonomy.admin.inc'),
    PATTERNS_EXPORT => array(
                          PATTERNS_EXPORT_ALL => 'taxonomy_patterns_export_all_vocs',
                          // Commented are more options for extraction
                          // This is not yet working
                          // 'Cheat (I am doing terms)' => 'taxonomy_patterns_export_all_terms',
                          //'Single Vocabulary' => array('callback' => 'taxonomy_patterns_export_single_vocabulary',
                          //                             'arguments' => array('1', '2', '3'),
                          //),
                        ),
  );
  
    $tags['d2d_permission'] = array(
	    PATTERNS_INFO   => t('Create/Modify/Delete Taxonomy Vocabularies'),
	    PATTERNS_CREATE => array('taxonomy_form_vocabulary'),
	    PATTERNS_MODIFY => array('taxonomy_form_vocabulary'),
	    PATTERNS_DELETE => array('taxonomy_vocabulary_confirm_delete'),
	    PATTERNS_FILES  => array('modules/taxonomy/taxonomy.admin.inc'),
	    PATTERNS_EXPORT => array(
	                          PATTERNS_EXPORT_ALL => 'taxonomy_patterns_export_all_vocs',
	                          // Commented are more options for extraction
	                          // This is not yet working
	                          // 'Cheat (I am doing terms)' => 'taxonomy_patterns_export_all_terms',
	                          //'Single Vocabulary' => array('callback' => 'taxonomy_patterns_export_single_vocabulary',
	                          //                             'arguments' => array('1', '2', '3'),
	                          //),
                        ),
  );
  
  
  return $tags;
  
}


function d2d_patterns_export_all_public_keys($args = NULL, &$result = NULL) {
	
	$action_type = PATTERNS_MODIFY; // pre-init 
	
	if (isset($args['type'])) {
      if ($args['type'] == PATTERNS_CREATE) {
      	$action_type = PATTERNS_CREATE;
      }
      // other action types
	}
	
  $info = d2d_patterns();
  $form_id = current($info['d2d_public_key'][PATTERNS_MODIFY]);
  $vocs = taxonomy_get_vocabularies();
  $result = array();
  foreach ($vocs as $voc) {
    $result = array_merge($result, patterns_export_actions_from_form($form_id, $voc, 'vocabulary', $action_type));
  }
  return $result;
}
