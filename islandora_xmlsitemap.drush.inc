<?php
/**
 * @file
 * Drush integration for the Islandora XMLsitemap module.
 */

/**
 * Implements hook_drush_command().
 *
 * @return array
 *   Associative array of commands.
 */
function islandora_xmlsitemap_drush_command() {
  $items['islandora_xmlsitemap_generate'] = array(
    'description' => 'Generate XMLsitemap records.',
    'arguments' => array(
      'max_chunk_size' => 'The number of records to grab each SOLR request',
      'limit' => 'The total number of records to grab before ending the process'
    ),
    'options' => array(
      'regenerate' => 'If set will clear the last_modified date before generating.'
    ),
  );
  return $items;
}

/**
 * XMLsitemap Generate command.
 */
function drush_islandora_xmlsitemap_generate($max_chunk_size = 100, $limit = null) {
  // Remove last_modified value if we need to regenerate.
  $regenerate = drush_get_option('regenerate', NULL);
  if (isset($regenerate)) {
    variable_del('islandora_xmlsitemap_last_modified_value');
  }

  // Start the process.
  module_load_include('inc', 'islandora_xmlsitemap', 'includes/batch');
  xmlsitemap_run_unprogressive_batch('islandora_xmlsitemap_get_batch', $max_chunk_size, $limit);
}
