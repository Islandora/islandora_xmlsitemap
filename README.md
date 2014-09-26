# Islandora XML sitemap [![Build Status](https://travis-ci.org/Islandora/islandora_xmlsitemap.png?branch=7.x)](https://travis-ci.org/Islandora/islandora_xmlsitemap)

## Introduction

Add URLs for Islandora objects to the XML sitemap module's database as custom links.  When the XML sitemap module creates its sitemap it will include these custom links.

## Requirements

This module requires the following modules and library:

* [Islandora](https://github.com/islandora/islandora)
* [xmlsitemap_custom](https://drupal.org/project/xmlsitemap) (Part of XML Sitemap)
* [islandora_solr](http://github.com/Islandora/islandora_solr_search)
* [Tuque](https://github.com/islandora/tuque)

## Installation

Install as usual, see [this](https://drupal.org/documentation/install/modules-themes/modules-7) for further information.

## Configuration

Set 'Last Modified Solr Field' and 'Maximum number of Islandora links to process at once' in Administration » Islandora » XML Sitemap Integration (admin/islandora/xmlsitemap).

![Configuration](http://i.imgur.com/EZSOKh7.png)

### Notes

Admins can configure the number of pids to process plus the Solr field to sort on.

To remove or edit links you can manage them in the XML sitemap Custom Links tab (admin/config/search/xmlsitemap/custom).

We have also implemented a number of hooks to automatically add/remove links to objects, including:

* `hook_islandora_object_purged()`
* `hook_islandora_object_ingested()`
* `hook_islandora_object_modified()`
* `hook_islandora_datastream_purged()`
* `hook_islandora_datastream_ingested()`
* `hook_islandora_datastream_modified()`

## Configuration

Set the paths for `example` and `module` in Administration » Islandora » MODULE (admin/islandora/module).

Include a screenshot of configuration page.

## Troubleshooting/Issues

Having problems or solved a problem? Check out the Islandora google groups for a solution.

* [Islandora Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora)
* [Islandora Dev Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora-dev)

## Maintainers/Sponsors

Current maintainers:

* [Paul Pound](https://github.com/ppound)
* [UPEI/Roberston Library](https://github.com/roblib)

## Development

If you would like to contribute to this module, please check out our helpful [Documentation for Developers](https://github.com/Islandora/islandora/wiki#wiki-documentation-for-developers) info, as well as our [Developers](http://islandora.ca/developers) section on the Islandora.ca site.

## License

[GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt)
