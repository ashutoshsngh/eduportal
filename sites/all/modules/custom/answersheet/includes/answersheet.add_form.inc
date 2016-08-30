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
      // '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
  );

  return $form;
}

function answersheet_add_form_validate($form, &$form_state) {
  $form_state['values']['files'] = $_FILES['files']['name'];
  unset($_FILES['files']);
}

function answersheet_add_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  $batch = array(
    'operations' => array(),
    'finished' => 'answersheet_batch_fix_finished',
    'title' => t('Upload Progress'),
    'init_message' => t('Upload is starting...'),
    'progress_message' => t('Processed @current out of @total.'),
    'error_message' => t('Fix has encountered an error.')
  );
  foreach ($form_state['values']['files'] as $result) {
    $batch['operations'][] = array('answersheet_batch_fix_process', array($result, $values));
  }
  batch_set($batch);
  batch_process('add/answer/sheet'); // The path to redirect to when done.
}

/**
 * Download answersheet callback.
 */
function download_answer_sheet() {
  db_set_active('legecy');
  $query = db_select('allocation_log', 'al');
  $query->fields('al')
      //->condition('al.head_examiner', 1)
      //->condition('al.downloadstatus', 0)
      ->groupBy('al.paper_code')
      ->groupBy('al.downloadstatus');
  $query = $query->extend('PagerDefault')->limit(50);
  $query->addExpression('SUM(al.total_sheets)', 'sumvalue');
  $result = $query->execute()->fetchAll();
  db_set_active();
  $header = array(
    array('data' => t('Paper Code')),
    array('data' => t('Allocated sheets')),
    array('data' => t('Action')),
  );
  $rows = array();
  if (!empty($result)) {
    foreach ($result as $value) {
      if ($value->downloadstatus) {
        $link = 'Already downloaded';
      }
      else {
        $link = l('Download', 'download/answer/sheet/' . $value->paper_code);
      }
      $rows[] = array(
        $value->paper_code,
        $value->sumvalue,
        $link,
      );
    }
  }
  $build = array();
  $build['table'] = array(
    '#markup' => theme('table', array('header' => $header, 'rows' => $rows, 'empty' => t('Nothing is assigned to you.'))),
  );
  $build['pager'] = array(
    '#markup' => theme('pager'),
  );
  return $build;
}

/**
 * Download answersheet in json format.
 * @param type $papercode
 */
function download_answer_sheet_json($papercode) {
  db_set_active('legecy');
  $query = db_select('answer_sheets', 'a');
  $query->fields('a')
      ->condition('a.status', 'ALLOCATED')
      //->condition('a.head_examiner', 1)
      //->condition('a.downloadstatus', 0)
      ->condition('a.paper_code', $papercode);
  $result = $query->execute()->fetchAll();
  db_set_active('default');
  $currentids = array_map(function ($value) {
    return $value->id;
  }, $result);
  $file = $papercode . '-answersheet1.json';
  file_put_contents('/tmp/' . $file, json_encode($result));
  header("Content-type: application/json");
  header('Content-Disposition: attachment; filename="' . basename($file) . '"');
  header('Content-Length: ' . filesize('/tmp/' . $file));
  header("refresh:1;");
  readfile('/tmp/' . $file);
  update_download_status($papercode, $currentids);
}

function update_download_status($papercode, $currentids) {
  db_set_active('legecy');
  $update = db_update('allocation_log');
  $update->fields(array(
    'downloadstatus' => 1,
  ));
  $update->condition('paper_code', $papercode, '=');
  $update->execute();
  $update = db_update('answer_sheets');
  $update->fields(array(
    'downloadstatus' => 1,
  ));
  $update->condition('id', $currentids, 'IN');
  $update->execute();
  db_set_active('default');
}
