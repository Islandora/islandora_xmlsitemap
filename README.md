# Islandora XML sitemap

## Description
Add urls for Islandora objects to the xmlsitemaps modules database as custom links.  When the xmlsitemap module creates it's site maps it will include these custom links.

### Notes
Admins can now configure the number of pids to process plus the solr field to sort on.

To remove or edit links you can manage them in the xmlsitemap custom links tab.

We have also implemented hook_islandora_object_ingested and
hook_islandora_object_purged to automatically add/remove links to objects
that have been ingested or purged.


### Requirements

* Drupal 7
* xmlsitemap_custom
* Islandora for drupal 7
* Islandora_solr_search

### TODO

We should probably should check for permission changes on links we encounter that
already exist in the link table.  It is possible for access to an object to change which
could make it unreachable to google scholar in which case we may want to remove the link.

We could probably also provide links to individual datastreams but I'm not sure what
priority to place on this and how we would want to implement this.

We may also be able to optimize the solr query so that we can elimate the access check
(not sure if we can do this and add the links to the datastreams, we may need to check
the datastream access anyway)

