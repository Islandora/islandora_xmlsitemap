# Islandora XML sitemap [![Build Status](https://travis-ci.org/Islandora/islandora_xmlsitemap.png?branch=7.x)](https://travis-ci.org/Islandora/islandora_xmlsitemap)

## Description
Add URLs for Islandora objects to the xmlsitemaps modules database as custom links.  When the xmlsitemap module creates its site maps it will include these custom links.

### Notes

Admins can now configure the number of pids to process plus the Solr field to sort on.

To remove or edit links you can manage them in the xmlsitemap custom links tab.

We have also implemented a number of hooks to automatically add/remove links to
objects, including:
* `hook_islandora_object_purged()`
* `hook_islandora_object_ingested()`
* `hook_islandora_object_modified()`
* `hook_islandora_datastream_purged()`
* `hook_islandora_datastream_ingested()`
* `hook_islandora_datastream_modified()`

### Requirements

* Drupal 7
  * xmlsitemap_custom which is part of [XML sitemap](https://drupal.org/project/xmlsitemap)
  * [islandora](http://github.com/Islanora/islandora)
  * [islandora_solr](http://github.com/Islandora/islandora_solr_search)

### TODO

We could probably provide links to individual datastreams but I'm not sure what
priority to place on this and how we would want to implement this.
