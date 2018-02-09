<?php
/**
 * @file
 * Drush integration for the Islandora XMLsitemap module.
 */

/**
 * Register the islandora_xmlsitemap Drush commands
 *
 * Implements hook_drush_command().
 *
 * @return array
 *   Associative array of commands.
 */
function islandora_xmlsitemap_drush_command() {
  $items['islandora_xmlsitemap_generate'] = array(
    'description' => 'Generate XMLsitemap records.',
    'options' => array(
      'max_chunk_size' => 'The number of records to grab each SOLR request',
      'limit' => 'The total number of records to grab before ending the process (defaults to no limit)',
      'regenerate' => 'If set will clear the last_modified date before generating.'
    )
  );
  return $items;
}

/**
 * Validate the input for the islandora_xmlsitemap_generate command.
 *
 * Implements drush_hook_COMMAND_validate().
 */
function drush_islandora_xmlsitemap_generate_validate() {
  // Validate max_chunk_size option
  $max_chunk_size = drush_get_option('max_chunk_size', NULL);
  if ($max_chunk_size !== NULL) {
    if (!islandora_xmlsitemap_is_integer_positive($max_chunk_size)) {
      return drush_set_error('The option "max_chunk_size" must be a positive integer greater than 0.');
    }
  }

  // Validate limit option
  $limit = drush_get_option('limit', NULL);
  if ($limit !== NULL) {
    if (!islandora_xmlsitemap_is_integer_positive($limit)) {
      return drush_set_error('The option "limit" must be a positive integer greater than 0.');
    }
  }

  // Validate regenerate option
  $regenerate = drush_get_option('regenerate', NULL);
  if ($regenerate !== NULL) {
    if ($regenerate !== TRUE) {
      return drush_set_error('The option "regenerate" can only be used without a value (--regenerate)');
    }
  }

  return TRUE;
}

/**
 * Execute the islandora_xmlsitemap_generate command.
 *
 * Implements drush_hook_COMMAND().
 */
function drush_islandora_xmlsitemap_generate() {
  // Remove last_modified value if we need to regenerate.
  $regenerate = drush_get_option('regenerate', NULL);
  if (isset($regenerate)) {
    variable_del('islandora_xmlsitemap_last_modified_value');
  }

  // Get the batch
  module_load_include('inc', 'islandora_xmlsitemap', 'includes/batch');
  $batch = islandora_xmlsitemap_get_batch(drush_get_option('max_chunk_size', 100), drush_get_option('limit', NULL));

  // Set and configure the batch.
  batch_set($batch);
  $batch =& batch_get();
  $batch['progressive'] = FALSE;

  // Start processing
  drush_backend_batch_process();
}

/**
 * Check if a (string) value is both not empty, an integer and positive.
 *
 * Validation logic taken from:
 * https://api.drupal.org/api/drupal/includes%21form.inc/function/element_validate_integer_positive/7.x
 *
 * @return boolean
 *   TRUE if $value is a positive integer, FALSE if $value is not a positive integer
 */
function islandora_xmlsitemap_is_integer_positive($value) {
    if ($value !== '' && (!is_numeric($value) || intval($value) != $value || $value <= 0)) {
      return FALSE;
    }
    return TRUE;
}
