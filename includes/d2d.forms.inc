<?php
/**
 * @file
 * Functions to create D2D forms or components of them
 */

function d2d_forms_select_friends(&$form = array(), $value = 'd2d_id', $display = 'url') {
	$friends = d2d_get_friends();
	
	$options = array();
	foreach($friends as $f) {
		$options[$f[$value]] = $f[$display];
	}
	
	$form += array(
		'#type' => 'select',
		'#options' => $options, 
	);
	
}
