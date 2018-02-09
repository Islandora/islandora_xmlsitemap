<?php
/**
 * @file
 * Batch definition and callbacks.
 */

/**
 * Get the our basic batch structure.
 *
 * @param int $chunk_size
 *   The number of records to grab each iteration of the operation.
 *
 * @param int $batch_limit
 *   The number of records to grab before ending the batch process.
 *
 * @return array
 *   An array representing the batch.
 */
function islandora_xmlsitemap_get_batch($max_chunk_size = 100, $batch_limit = NULL){
  $mod_path = drupal_get_path('module', 'islandora_xmlsitemap');
  $batch = array(
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaining @estimate.'),
    'operations' => array(
      // Using a single operation that will be called multiple times
      array('islandora_xmlsitemap_batch_operation', array($max_chunk_size, $batch_limit)),
    ),
    'file' => "$mod_path/includes/batch.inc",
    'finished' => 'islandora_xmlsitemap_finished'
  );
  return $batch;
}

/**
 * Batch operation. Grab a number of records from Solr at a time.
 *
 * This operation will be called multiple times until batch limit is reached or
 * there are no records newer than the last_modified date that was defined.
 *
 * Gets variables:
 * - islandora_xmlsitemap_last_modified_value (starting point for query)
 * - islandora_xmlsitemap_last_modified_field (field to look for
 *    last_modified_value to match against)
 * - islandora_namespace_restriction_enforced
 *
 * Sets variable:
 * - islandora_xmlsitemap_last_modified_value
 *
 * @param int $chunk_size
 *   The number of records to grab each iteration of the operation.
 *
 * @param int $batch_limit
 *   The number of records to grab before ending the batch process.
 *
 * @param array &$context
 *   Reference to persistent context of the operation.
 */
function islandora_xmlsitemap_batch_operation($max_chunk_size = 100, $batch_limit = null, &$context){
  // Create sandbox alias
  $sandbox =& $context['sandbox'];

  /*
   * Sandbox setup (only on first call of operation).
   */
  if (empty($sandbox)) {
    $sandbox = array();

    // The amount of rows processed.
    $sandbox['progress']      = 0;

    // The PID of the latest processed row.
    $sandbox['current_node']  = "None";

    // Represents total to be retrieved during this batch process. Set after initial results are retrieved.
    $sandbox['total']         = null;

    // Set a batch limit if specified
    if (isset($batch_limit)) {
      $sandbox['batch_limit'] = (int) $batch_limit;
    }

    // Boostrap an IslandoraSolrQueryProcessor for our purposes.
    $last_modified = $sandbox['last_modified']  = variable_get('islandora_xmlsitemap_last_modified_value', NULL);
    $last_modified_field                        = variable_get('islandora_xmlsitemap_last_modified_field', 'fgs_lastModifiedDate_dt');
    $enforce_namespaces                         = variable_get('islandora_namespace_restriction_enforced', FALSE);
    $sandbox['queryProcessor']                  = islandora_xmlsitemap_queryProcessor_create($last_modified_field, $last_modified, $enforce_namespaces);

    // Set maximum chunk size.
    $sandbox['max_chunk_size'] = (int) $max_chunk_size;

    // Set default SOLR limit.
    $sandbox['queryProcessor']->solrLimit = $sandbox['max_chunk_size'];
  }

  /*
  * Iteration start.
  */

  // Set SOLR offset to where we left off in the previous iteration.
  $sandbox['queryProcessor']->solrStart = $sandbox['progress'];

  // Set chunk size for limited batches. This prevents overshooting
  // the batch_limit.
  if (isset($sandbox['batch_limit'])) {
    // Max chunk size for limited batches.
    $sandbox['max_chunk_size'] = min($sandbox['batch_limit'] - $sandbox['progress'], $sandbox['max_chunk_size']);

    // Update SOLR limit.
    $sandbox['queryProcessor']->solrLimit = $sandbox['max_chunk_size'];
  }

  // Execute SOLR query and get result.
  $sandbox['queryProcessor']->executeQuery(FALSE, TRUE);
  $result       = $sandbox['queryProcessor']->islandoraSolrResult;
  $resultCount  = (int) $result['response']['numFound'];

  // Result will be null in case of SOLR error. The queryprocessor will have set
  // an error using drupal_set_message.
  $hasSolrError = ($result === NULL);

  // Catch a result that does not have any rows
  $noResults = ($resultCount === 0);

  // Check if we encountered error conditions
  if ($hasSolrError || $noResults) {
    // SOLR error
    if ($hasSolrError) {
      // Set message indicating a SOLR error.
      $context['message'] = t('SOLR error');
    }

    // No results
    if ($noResults) {
      // Set message indicating no results.
      $context['message'] = t('No results');
    }

    // Add error to the result list.
    $context['results'][] = $context['message'];

    // Set finished to 1 so the batch process will not loop endlessly.
    $context['finished'] = 1;

    // Return control to batch engine.
    return;
  }

  // Set total based on results (only once every batch).
  if (!isset($sandbox['total'])) {
    if (isset($sandbox['batch_limit'])) {
      // Limited batch total is batch_limit or resultcount if there are less
      // results than the batch_limit.
      $sandbox['total'] = min($resultCount, $sandbox['batch_limit']);
    }else{
      // Unlimited batch total is equal to the number of results.
      $sandbox['total'] = $resultCount;
    }
  }

  // Process result rows.
  module_load_include('inc', 'islandora_xmlsitemap', 'includes/utilities');
  $resultRows = $result['response']['objects'];
  foreach ($resultRows as $row) {
    // Add or Update link.
    islandora_xmlsitemap_add_or_update_link($row['PID'], $sandbox);

    // Update our progress information.
    $sandbox['progress']++;
    $sandbox['current_node'] = $row['PID'];

    // Log result for post-processing in the 'finished' callback.
    $context['results'][] = "Add/update " . $sandbox['current_node'];
  }
  //Drop results when finished with them.
  $sandbox['queryProcessor']->resetResults();

  // Set message to be returned.
  $context['message'] = t('Processed @count of @total. Current node: @pid.', array(
    '@count' => $sandbox['progress'],
    '@total' => $sandbox['total'],
    '@pid' => $sandbox['current_node']
  ));

  // Persist the last processed records last modified date.
  // This allows facilitates continuing from this point onward next time in case
  // not all records have been processed due to limits set or a crash.
  if (isset($sandbox['last_modified'])) {
    variable_set('islandora_xmlsitemap_last_modified_value', $sandbox['last_modified']);
  }

  // Inform the batch engine of our progress,
  // and provide an estimation of the completion level we reached.
  $context['finished'] = $sandbox['progress'] / $sandbox['total'];
}

/**
 * Bootstraps a new queryprocessor for Islandora XMLsitemap.
 *
 * @param string $last_modified_field
 *  The date field to use for sorting and filtering.
 *
 * @param string $last_modified
 *  The last modified date.
 *
 * @param boolean $enforce_namespaces
 *  Enforce Islandora namespaces or not. If enabled allowed namespaces will be
 *  retrieved using islandora_get_allowed_namespaces(). Defaults to FALSE.
 *
 * @return IslandoraSolrQueryProcessor Bootstrapped SOLR query processor.
 */
function islandora_xmlsitemap_queryProcessor_create($last_modified_field, $last_modified = NULL, $enforce_namespaces = FALSE) {
  // Instantiate Islandora Query Processor.
  $qp = new IslandoraSolrQueryProcessor();

  // Set sorting on the $last_modified_field field, ascending so we go from older to newer records.
  $qp->solrParams['sort'] = "$last_modified_field asc";

  // Set empty filter query.
  $qp->solrParams['fq'] = array();

  // Set or update the latest last_modified value.
  $qp->solrQuery = islandora_xmlsitemap_queryProcessor_last_modified_query(
    $last_modified_field,
    $last_modified
  );

  // Return only the PID field.
  $qp->solrParams['fl'] = 'PID';

  // Enforce namespace if required by settings.
  if ($enforce_namespaces) {
    $namespace_map = function ($namespace) {
      return 'PID:' . Apache_Solr_Service::escape("$namespace:") . '*';
    };
    module_load_include('inc', 'islandora', 'includes/utilities');
    $qp->solrParams['fq'][] = implode(' OR ', array_map($namespace_map, islandora_get_allowed_namespaces()));
  }

  return $qp;
}

/**
 * Creates a SOLR query string based on the last modified value.
 *
 * @param string $last_modified_field Last modified date field.
 * @param string $last_modified Last modified date.
 *
 * @return string SOLR query string.
 */
function islandora_xmlsitemap_queryProcessor_last_modified_query($last_modified_field, $last_modified = NULL) {
  if (isset($last_modified)) {
    return "$last_modified_field:[{$last_modified} TO *]";
  }
  return "$last_modified_field:[* TO *]";
}

/**
 * Finished callback.
 *
 * Displays result or errors when the batch finishes.
 */
function islandora_xmlsitemap_finished($success, $results, $operations) {
  if ($success) {
    // Display the result.
    drupal_set_message(t('@count results', array('@count' => count($results))));
    drupal_set_message(t('The last result was "%final"', array('%final' => end($results))));
  }
  else {
    // Display an error.
    drupal_set_message(
      t(
        'An error occurred while processing. The last result was: %final. Batch arguments: @args',
        array(
          '%final' => end($results),
          '@args' => print_r($error_operation[0], TRUE)
        )
      ),
      'error'
    );
  }
}
