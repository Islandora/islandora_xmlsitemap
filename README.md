# Islandora XML sitemap

## Description
Add urls for Islandora objects to the xmlsitemaps modules database as custom links.  When the xmlsitemap module creates it's site maps it will include these custom links.

### Notes
Admins can now configure the number of pids to process plus the solr field to sort on.

To remove or edit links you can manage them in the xmlsitemap custom links tab.  We have implemented
hook_islandora_object_purged($pid) to automatically remove links to objects that have been purged from Fedora.

### Requirements

* Drupal 7
* xmlsitemap_custom
* Islandora for drupal 7
* Islandora_solr_search

