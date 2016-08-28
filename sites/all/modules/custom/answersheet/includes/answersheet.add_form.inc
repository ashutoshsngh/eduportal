<?php

/**
 * @file
 * Answersheet add form.
 */

/**
 * Callback function for answersheet add form.
 */
function answersheet_add_form() {
  drupal_add_js(drupal_get_path('module', 'answersheet') . '/answersheet.js');
  $options = get_submitted_paper_options();
  db_set_active('default');
  $form['select_paper'] = array(
    '#type' => 'select',
    '#options' => $options,
    '#title' => t('Select Paper'),
    '#description' => t('Subject|Region|Set|Language'),
    '#required' => TRUE,
  );

  $form['exam_code'] = array(
    '#type' => 'markup',
    '#markup' => '<div id="exam_code_wrapper" style="display:none;">Exam Code : <div id="exam_code_value"></div></div>',
  );

  $form['file'] = array(
    '#type' => 'file',
    '#name' => 'files[]',
    '#title' => t('Upload answer sheets'),
    '#attributes' => array('multiple' => 'multiple'),
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
  );

  return $form;
}

function answersheet_add_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  $batch = array(
    'operations' => array(),
    'finished' => 'answersheet_batch_fix_finished',
    'title' => t('Batch fix'),
    'init_message' => t('Upload is starting...'),
    'progress_message' => t('Processed @current out of @total.'),
    'error_message' => t('Fix has encountered an error.')
  );
  $results = array("Hello", "World");
  foreach ($_FILES['files']['name'] as $result) {
    $batch['operations'][] = array('answersheet_batch_fix_process', array($result, $values));
  }
  batch_set($batch);
  batch_process('add/answer/sheet'); // The path to redirect to when done.
}
