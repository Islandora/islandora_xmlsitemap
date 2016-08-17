<?php

/**
 * @file
 * Functionality that touches the database.
 */

/**
 * Inserts a record into the autoincrementing ID table.
 *
 * @param string $value
 *   The value to insert into the 'pid' column.
 */
function islandora_xmlsitemap_insert_increment_record($value) {
  db_insert('islandora_xmlsitemap_entry_ids')
    ->fields(array('pid' => $value))
    ->execute();
}

/**
 * Deletes a record from the autoincrementing ID table.
 *
 * @param string $value
 *   The value to match in the 'pid' column for record deletion.
 */
function islandora_xmlsitemap_delete_increment_record($value) {
  db_delete('islandora_xmlsitemap_entry_ids')
    ->condition('pid', $value)
    ->execute();
}

/**
 * Gets an incremented ID from the autoincrementing table.
 *
 * Does so by entering a row, querying for that record, and then immediately
 * deleting that row.
 *
 * @param string $value
 *   A unique value to use in the 'pid' column to ensure a unique incremented
 *   ID.
 *
 * @return int
 *   A unique, incremented ID from the table.
 */
function islandora_xmlsitemap_get_incremented_id($value) {
  islandora_xmlsitemap_insert_increment_record($value);
  $id = db_select('islandora_xmlsitemap_entry_ids', 'x')
    ->fields('x', array('id'))
    ->condition('pid', $value)
    ->execute()
    ->fetchField();
  islandora_xmlsitemap_delete_increment_record($value);
  return $id;
}
