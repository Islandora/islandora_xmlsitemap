<?php
/**
 * @file
 * include file for the islandora_xmlsitemap module.
 */

/**
 * Entry point for batch creation.
 * @return array
 *   the batch array including operations
 */
function islandora_xmlsitemap_update_customlinks() {
  $operations = array();
  $pids = islandora_xmlsitemap_get_objects();
  if (!empty($pids)) {
    foreach ($pids as $result) {
      $operations[] = array(
        'islandora_xmlsitemap_add_link',
        array($result['PID'])
      );
    }
  }
  return array(
    'operations' => $operations,
    'finished' => 'Finished adding Custom Links'
  );
}

/**
 * Gets a batch of pids to process.
 *
 * uses variable_get to to get the starting number and the number of objects
 * to process.
 *
 * @return array
 *   an array of pids
 */
function islandora_xmlsitemap_get_objects() {
  $namespaces_enforced = variable_get('islandora_namespace_restriction_enforced', FALSE);
  $namespaces = variable_get('islandora_pids_allowed');
  $start = variable_get('islandora_xmlsitemap_start', 0);
  $num_to_process = variable_get('islandora_xmlsitemap_number_of_pids_to_process', 10);
  $end = $start + $num_to_process;
  $results = islandora_xmlsitemap_solr_get_results($namespaces_enforced, $namespaces, $start, $num_to_process);
  if ($results['response']['numFound'] > $end) {
    variable_set('islandora_xmlsitemap_start', $end);
  }
  else {
    variable_set('islandora_xmlsitemap_start', 0);
  }
  $finish = ($end < $results['response']['numFound']) ? $end : $results['response']['numFound'];
  watchdog('Islandora_xmlsitemap', 'found %count islandora objects, processing %start to %finish',
    array(
      '%count' => $results['response']['numFound'],
      '%start' => $start,
      '%finish' => $finish
    ));
  return $results['response']['docs'];
}


/**
 * Queries solr for a list of pids.
 *
 * If namespaces are enabled it will filter by namespace
 *
 * @param bool $namespaces_enforced
 *   TRUE if namespace restrictions are enforced
 * @param string $namespaces
 *   The list of namespaces to use if namespace restrictrictions are enforced.
 *
 * @param int $start
 *   start with result
 * @param int $end
 *   number of results to fetch
 *
 * @return array|null
 *   an array of solr results
 *   $results['response']['docs']['pid']
 */
function islandora_xmlsitemap_solr_get_results($namespaces_enforced, $namespaces,
                                               $start, $end) {
  $solr_location = variable_get('islandora_solr_url', 'http://localhost:8080/solr');
  $solr_handler = variable_get('islandora_solr_request_handler', FALSE);
  $path_parts = parse_url(variable_get('islandora_solr_url', 'localhost:8080/solr'));
  $solr_query = 'PID:*';
  $solr = new Apache_Solr_Service($path_parts['host'], $path_parts['port'], $path_parts['path'] . '/');
  $solr->setCreateDocuments(0);
  $solr_params = array();
  $solr_params['fl'] = array('PID');
  $solr_params['sort'] = array(variable_get('islandora_xmlsitemap_sort_field', 'fgs_createdDate_dt') . ' asc');
  if ($solr_handler) {
    $solr_params['qt'] = $solr_handler;
  }
  if ($namespaces_enforced) {
    $namespace_list = trim(variable_get('islandora_pids_allowed', ''));
    if ($namespace_list) {
      $namespaces = preg_split('/[,|\s]/', $namespace_list);
      $namespace_array = array();
      foreach (array_filter($namespaces) as $namespace) {
        $namespace_array[] = 'PID:' . Apache_Solr_Service::escape($namespace) . '*';
      }
      $solr_params['fq'][] = implode(' OR ', $namespace_array);
    }
  }

  try {
    $method = 'GET';
    $results = $solr->search($solr_query, $start, $end, $solr_params, $method);
  }
  catch (Exception $e) {
    watchdog('islandora_xmlsitemap', 'Error getting solr results %e',
      array('%e' => $e->getMessage()));
  }
  $solr_results = NULL;
  if (!empty($results)) {
    $solr_results = json_decode($results->getRawResponse(), TRUE);
  }
  return $solr_results;
}

/**
 * Adds a link to the xmlsitemap database.
 *
 * @param String $pid
 *   A Fedora objects pid
 * @param AbstractFedora $object
 *   Add a link to the sitemap for this object
 *
 * @return array|null
 */
function islandora_xmlsitemap_add_link($pid, $object = NULL) {
  $data = ISLANDORA_XMLSITEMAP_PATH_PREFIX . "$pid";
  $return = NULL;
  // When running from cron $object might be not null but not actually what we want.
  if (!isset($object) || !($object instanceof AbstractFedoraObject)) {
    $object = islandora_object_load($pid);
  }
  $link = array(
    'type' => 'custom',
    'loc' => $data,
    'changefreq' => XMLSITEMAP_FREQUENCY_MONTHLY,
    'priority' => XMLSITEMAP_PRIORITY_DEFAULT,
    // 'lastmod' => $object->lastModifiedDate->format('Y-m-d'),
  );
  if (islandora_xmlsitemap_check($link, $object)) {
    $link['id'] = islandora_xml_sitemap_get_next_id();
    $return = xmlsitemap_link_save($link);
  }
  return $return;
}

/**
 * Queries the database to get the last id.
 *
 * @return int
 *   the next id for the xmlsitemap link
 */
function islandora_xml_sitemap_get_next_id() {
  return db_query("SELECT MAX(id) FROM {xmlsitemap} WHERE type = 'custom'")->fetchField() + 1;
}


/**
 * Verify that we are allowed to add this link to the sitemap db.
 *
 * @param array $link
 *   the link array containing link['loc'] (the uri to check)
 *
 * @param FedoraObject $object
 *   the object we want to test for access as the anonymous user
 *
 * @return bool
 *   returns true if the link doesn't already exist in the db and
 * anonymous users can access the link
 */
function islandora_xmlsitemap_check($link, $object) {
  $results = db_query_range("SELECT 1 FROM {xmlsitemap} WHERE type = 'custom'
  AND loc = :loc", 0, 1, array(
    ':loc' => $link['loc']
  ))->fetchAll();
  if ($results) {
    return FALSE;
  }
  $anon_user = & drupal_static(__FUNCTION__);
  if (!isset($anon_user)) {
    $anon_user = drupal_anonymous_user();
  }
  return islandora_user_access($object, array(FEDORA_VIEW_OBJECTS), array(), TRUE, $anon_user);
}
