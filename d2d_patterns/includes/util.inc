<?php

// TODO add _patterns and update methods calls

function d2d_api_public_key_exist($d2d_id, $public_key) {
  return d2d_api_public_key_return_id($d2d_id, $public_key) !== FALSE;
}