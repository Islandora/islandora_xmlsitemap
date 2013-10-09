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
  $pids = islandora_xmlsitemap_get_all_objects();
  foreach ($pids as $pid) {
    $operations[] = array('islandora_xmlsitemap_add_link', $pid);
  }
  return array(
    'operations' => $operations,
    'finished' => 'Finished adding Custom Links'
  );
}

/**
 * Performs an itql query that returns all islandora objects.
 *
 * The objects are limited to children of the root collection for this site
 *
 * @return array
 *   an array of pids
 */
function islandora_xmlsitemap_get_all_objects() {
  $root_collection = variable_get('fedora_repository_pid', 'islandora:root');
  $itql_query = <<<ITQLQUERY
  select \$object from <#ri>
  where walk(
    \$collection <info:fedora/fedora-system:def/relations-external#isMemberOfCollection><info:fedora/$root_collection>
  and \$object <info:fedora/fedora-system:def/relations-external#isMemberOfCollection> \$collection)
ITQLQUERY;

  module_load_include('inc', 'fedora_repository', 'ObjectHelper');
  return ObjectHelper::performRiQuery($itql_query);
}

/**
 * Adds a link to the xmlsitemap database.
 *
 * @param String $pid
 *   A Fedora objects pid
 */
function islandora_xmlsitemap_add_link($pid) {
  $data = "fedora/repository/$pid";
  $link = array(
    'type' => 'custom',
    'loc' => $data,
    'changefreq' => XMLSITEMAP_FREQUENCY_MONTHLY,
    'priority' => XMLSITEMAP_PRIORITY_DEFAULT,
  );
  $ok_to_add = islandora_xmlsitemap_check($link);
  if ($ok_to_add) {
    $link['id'] = islandora_xml_sitemap_get_next_id();
    xmlsitemap_link_save($link);
  }
  islandora_xml_sitemap_add_fulltext_link($link, $pid);
}


/**
 * Add the fulltext link to the xmlsitemap database if there is fulltext.
 *
 * Checks to make sure that we have access and it is not embargoed.  Relies
 * on the UPEI version of the scholar module if the scholar module is not
 * enabled it will return false
 *
 * @param array $link
 *   array containing the links uri, type, priority, changefreq
 * @param string $pid
 *   the pid of the fedora object
 *
 * @return mixed
 *   return FALSE if we did not add a link to the db other wise return the
 * link array
 */
function islandora_xml_sitemap_add_fulltext_link($link, $pid) {
  if (!module_exists('scholar')) {
    return FALSE;
  }
  module_load_include('inc', 'scholar', 'Coins');
  xmlsitemap_switch_user(0);
  $c = new Coins($pid);
  $datastream_array = $c->object->get_datastreams_list_as_array();
  $full_text_uri = $c->get_download_url($datastream_array, FALSE);
  xmlsitemap_restore_user();
  if (!empty($full_text_uri)) {
    $link['loc'] = $full_text_uri;
    if (islandora_xmlsitemap_check($link)) {
      $link['id'] = islandora_xml_sitemap_get_next_id();
      $return_value = xmlsitemap_link_save($link);
      $c->object->forget();
      return $return_value;
    }
  }
  return FALSE;
}

/**
 * Queries the database to get the last id.
 *
 * @return int
 *   the next id for the xmlsitemap link
 */
function islandora_xml_sitemap_get_next_id() {
  return db_result(db_query("SELECT MAX(id) FROM {xmlsitemap} WHERE type = 'custom'")) + 1;
}


/**
 * Verify that we are allowed to add this link to the sitemap db.
 *
 * @param array $link
 *   the link array containing link['loc'] (the uri to check)
 *
 * @return bool
 *   returns true if the link doesn't already exist in the db and
 * anonymous users can access the link
 */
function islandora_xmlsitemap_check($link) {
  if (db_result(db_query_range("SELECT 1 FROM {xmlsitemap} WHERE type = '%s' AND loc = '%s'", $link['type'], $link['loc'], 0, 1))) {
    return FALSE;
  }
  // TODO: we can probably make the switch to anonymous more efficient.
  // In D7 version we can use the islandora perm functions to check access
  // for now just do menu check as in d6 our access checks have changed over
  // time and some of our older sites don't have the preferred functions in
  // there islandora versions.
  xmlsitemap_switch_user(0);
  $menu_item = menu_get_item($link['loc']);
  xmlsitemap_restore_user();
  if (!$menu_item['access']) {
    return FALSE;
  }
  if ($menu_item && strpos($link['loc'], 'admin/settings/xmlsitemap/custom') === 0 && !user_access('administer xmlsitemap', drupal_anonymous_user())) {
    return FALSE;
  }
  return TRUE;
}