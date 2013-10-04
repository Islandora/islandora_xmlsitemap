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
    $link['id'] = db_result(db_query("SELECT MAX(id) FROM {xmlsitemap} WHERE type = 'custom'")) + 1;
    $link['lastmod'] = time();
    xmlsitemap_link_save($link);
  }
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