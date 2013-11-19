<?php

/**
 * @file
 * Admin form and handlers.
 */

/**
 * For building function.
 */
function islandora_xmlsitemap_admin_form($form, &$form_state) {
  form_load_include($form_state, 'inc', 'islandora_xmlsitemap', 'includes/admin.form');

  $form['#tree'] = TRUE;

  $last_modified_value = variable_get('islandora_xmlsitemap_last_modified_value', NULL);
  $form['islandora_xmlsitemap_last_modified_value'] = array(
    '#type' => 'item',
    '#title' => t('Last Modified Value'),
    '#access' => $last_modified_value !== NULL,
    '#description' => t('The last modified date of the object which was last added to the xmlsitemap table.'),
    'value' => array(
      '#type' => 'item',
      '#markup' => $last_modified_value,
    ),
    'reset_value' => array(
      '#type' => 'submit',
      '#value' => t('Reset last modified date'),
    ),
  );
  $form['islandora_xmlsitemap_last_modified_field'] = array(
    '#type' => 'textfield',
    '#title' => t('Last Modified Solr Field'),
    '#description' => t('Solr field in which we can perform sorting and range queries.'),
    '#default_value' => variable_get('islandora_xmlsitemap_last_modified_field', 'fgs_lastModifiedDate_dt'),
  );
  $form['islandora_xmlsitemap_number_of_pids_to_process'] = array(
    '#type' => 'textfield',
    '#title' => 'Maximum Number of Islandora links to process at once',
    '#size' => 10,
    '#element_validate' => array('element_validate_integer_positive'),
    '#default_value' => variable_get('islandora_xmlsitemap_number_of_pids_to_process', 1000),
    '#description' => 'This is the number of Islandora/Fedora links we will process at once',
  );

  $form['actions'] = array(
    '#type' => 'actions',
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Save configuration'),
    ),
    'reset' => array(
      '#type' => 'submit',
      '#value' => t('Reset configuration'),
    ),
  );

  return $form;
}

/**
 * Form submission handler.
 */
function islandora_xmlsitemap_admin_form_submit(&$form, &$form_state) {
  $button = end($form_state['clicked_button']['#parents']);

  $to_set = array(
    'islandora_xmlsitemap_last_modified_field',
    'islandora_xmlsitemap_number_of_pids_to_process',
  );
  if ($button == 'submit') {
    foreach ($to_set as $var) {
      variable_set($var, $form_state['values'][$var]);
    }
    drupal_set_message(t('Configuration saved!'));
  }
  else {
    if ($button == 'reset') {
      foreach ($to_set as $var) {
        variable_del($var);
      }
    }
    variable_del('islandora_xmlsitemap_last_modified_value');
    drupal_set_message(t('Configuration reset.'));
  }
}
