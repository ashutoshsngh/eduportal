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
    '#upload_validators' => array(
      'file_validate_extensions' => array('pdf'),
    ),
      // '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
  );

  return $form;
}

function answersheet_add_form_validate($form, &$form_state) {
  $array_unique = array_filter($_FILES['files']['type']);
  if(count($array_unique) > 1) {
    form_set_error('','Only pdf files are accepted.');
  }
  if($array_unique[0] != 'text/pdf') {
    form_set_error('','Only pdf files are accepted.');
  }
}

function answersheet_add_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  $selectedpaper = explode('-', $values['select_paper']);

  $batch = array(
    'operations' => array(),
    'finished' => 'answersheet_batch_fix_finished',
    'title' => t('Upload Progress'),
    'init_message' => t('Upload is starting...'),
    'progress_message' => t('Processed @current out of @total.'),
    'error_message' => t('Fix has encountered an error.')
  );
  $s3 = initialize_aws_connection();
  $bucket = variable_get('bucket_name');
  foreach ($_FILES['files']['name'] as $key => $value) {
    $result = $s3->putObject(array(
      'Bucket' => $bucket,
      'Key' => $selectedpaper[5] . '/' . $value,
      'SourceFile' => $_FILES['files']['tmp_name'][$key],
      'ACL' => 'public-read',
      'StorageClass' => 'REDUCED_REDUNDANCY',
    ));
    //Check if file is successfully uploaded on aws server.
    if ($result['@metadata']['statusCode'] == 200) {
      $batch['operations'][] = array('answersheet_batch_fix_process', array($value, $values, $selectedpaper));
    }
  }
  batch_set($batch);
  batch_process('add/answer/sheet'); // The path to redirect to when done.
}

/**
 * Download answersheet callback.
 */
function download_answer_sheet() {
  global $user;
  $keys = array_keys($user->roles);
  $loggedinuser = get_userdata($user->mail);
  db_set_active('legecy');
  $query = db_select('allocation_log', 'al');
  $query->fields('al');
  if(in_array(EXTERNAL_HEAD_EXAMINER_GLOBAL_RID,$keys)) {
    $query->condition('al.head_examiner',$loggedinuser['id']);
  }
  $query->condition('al.downloadstatus', 0);
  $query->groupBy('al.paper_code')
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
  global $user;
  $keys = array_keys($user->roles);
  $loggedinuser = get_userdata($user->mail);
  db_set_active('legecy');
  $query = db_select('answer_sheets', 'a');
  $query->fields('a')
      ->condition('a.status', 'ALLOCATED');
  if(in_array(EXTERNAL_HEAD_EXAMINER_GLOBAL_RID,$keys)) {
    $query->condition('a.head_examiner',$loggedinuser['id']);
  }
  $query->condition('a.downloadstatus', 0);
  $query->condition('a.paper_code', $papercode);
  $result = $query->execute()->fetchAll();
  db_set_active('default');
  $s3 = initialize_aws_connection();
  $bucket = variable_get('bucket_name');
  $foldername = '/tmp/package-' . time();
  $answersheetfolder = $foldername . '/answersheets';
  mkdir($foldername);
  mkdir($answersheetfolder);
  $logid = array();
  foreach ($result as $key=> $value) {
    $logid[$key] = $value->allocation_log_id;
    $fileexist = $s3->doesObjectExist($bucket, $papercode.'/' . $value->sheet_name);
    if ($fileexist) {
      $response = $s3->getObject(array(
        'Bucket' => $bucket,
        'Key' => $papercode.'/' . $value->sheet_name,
        'SaveAs' => $answersheetfolder . '/' . $value->sheet_name
      ));
    }
  }
  $logdetails = get_log_details(array_unique($logid));
  $currentids = array_map(function ($value) {
    return $value->id;
  }, $result);
  $file = $papercode . '-answersheet.json';
  $log = $papercode.'-allocationlog.json';
  file_put_contents($foldername . '/' . $file, json_encode($result));
  file_put_contents($foldername . '/' . $log, json_encode($logdetails));
  createZipfile($foldername);
  $archive_file_name = $foldername . '.zip';
  // then send the headers to force download the zip file
  header("Content-type: application/zip");
  header('Content-Disposition: attachment; filename="' . basename($archive_file_name) . '"');
  header("Refresh:1;");
  readfile("$archive_file_name");
  update_download_status($papercode, $currentids);
}

function createZipfile($filepath) {
  // Get real path for our folder
  $rootPath = realpath($filepath);

// Initialize archive object
  $zip = new ZipArchive();
  $zip->open($filepath . '.zip', ZipArchive::CREATE | ZipArchive::OVERWRITE);

// Create recursive directory iterator
  /** @var SplFileInfo[] $files */
  $files = new RecursiveIteratorIterator(
      new RecursiveDirectoryIterator($rootPath), RecursiveIteratorIterator::LEAVES_ONLY
  );

  foreach ($files as $name => $file) {
    // Skip directories (they would be added automatically)
    if (!$file->isDir()) {
      // Get real and relative path for current file
      $filePath = $file->getRealPath();
      $relativePath = substr($filePath, strlen($rootPath) + 1);

      // Add current file to archive
      $zip->addFile($filePath, $relativePath);
    }
  }

// Zip archive will be created only after closing object
  $zip->close();
}

function update_download_status($papercode, $currentids) {
  global $user;
  $keys = array_keys($user->roles);
  $loggedinuser = get_userdata($user->mail);
  db_set_active('legecy');
  $update = db_update('allocation_log');
  $update->fields(array(
    'downloadstatus' => 1,
  ));
  $update->condition('paper_code', $papercode, '=');
  if(in_array(EXTERNAL_HEAD_EXAMINER_GLOBAL_RID,$keys)) {
    $update->condition('head_examiner',$loggedinuser['id']);
  }
  $update->execute();
  if(!empty($currentids)) {
  $update = db_update('answer_sheets');
  $update->fields(array(
    'downloadstatus' => 1,
  ));
  $update->condition('id', $currentids, 'IN');
  $update->execute();
  }
  db_set_active('default');
}
