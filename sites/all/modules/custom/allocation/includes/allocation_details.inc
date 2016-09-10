<?php

/**
 * @file
 * Inc file for contact us admin settings.
 */

/**
 * Callback function for contact us settings form.
 */
function allocation_details_entry($forn, &$form_state, $code) {
  global $user;
  $keys = array_keys($user->roles);
  //print_r($keys);exit;
  $form = array();
  $form['papercode_label'] = array(
    '#type' => 'item',
    '#markup' => 'New Allocation for Paper Code : ' . $code,
  );
  $form['allocation_papercode'] = array(
    '#type' => 'hidden',
    '#default_value' => $code,
  );
  $form['allocation_centercode'] = array(
    '#type' => 'hidden',
    '#default_value' => 'CNR9U8',
  );
  $form['allocated_by'] = array(
    '#type' => 'hidden',
    '#default_value' => $user->name,
  );
  if(in_array(5, $keys)) {
      $form['he_status'] = array(
          '#type' => 'hidden',
          '#value' => 1,
      );
      $total_allocation_count = answersheet_unallocated_count($code,1);
  }else{
      $total_allocation_count = answersheet_unallocated_count($code);
      $form['he_status'] = array(
          '#type' => 'hidden',
          '#value' => 0,
      );
  }
  
  $form['unallocated_count'] = array(
    '#type' => 'hidden',
    '#default_value' => $total_allocation_count,
  );
  
  $form['total_allocation_label'] = array(
    '#type' => 'item',
    '#markup' => 'Total Unallocated : '.$total_allocation_count,
  );

  $question_details = questionpaper_details($code);
  $subject = subject_load($question_details[0]->subject_id);

  $form['subject_label'] = array(
    '#type' => 'item',
    '#markup' => 'Subject : ' . $subject->subject_name,
  );

  $region = region_load($question_details[0]->region_id);
  $form['region_label'] = array(
    '#type' => 'item',
    '#markup' => 'Region : ' . $region->region_name,
  );
  $form['paperset_label'] = array(
    '#type' => 'item',
    '#markup' => 'Set : ' . $question_details[0]->paper_set,
  );
  $form['medium_label'] = array(
    '#type' => 'item',
    '#markup' => 'Medium : ' . $question_details[0]->language,
  );
  $form['no_sheet_allocated'] = array(
    "#type" => 'textfield',
    "#title" => t("Number of sheet to be allocated"),
    '#required' => TRUE,
  );
  if(in_array(5, $keys)) {
      $form['head_examiner'] = array(
    '#type' => 'select',
    "#title" => t("Select Examiner"),
    '#options' => exam_portal_all_examiner(),
    '#required' => TRUE,
  );
      
  }else{
   $form['head_examiner'] = array(
    '#type' => 'select',
    "#title" => t("Select Head Examiner"),
    '#options' => exam_portal_all_head_examiner(),
    '#required' => TRUE,
  );   
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Allocation'),
  );
  return $form;
}

/**
 * Validate handler allocation_details_entry().
 */
function allocation_details_entry_validate($form, &$form_state) {
    $values = $form_state['values'];
    if($values['unallocated_count'] < $values['no_sheet_allocated']) {
        form_set_error('no_sheet_allocated',t('Sheets allocated cannot be greater then remaining sheets.'));
    }
}

/**
 * Submit handler allocation_details_entry().
 */
function allocation_details_entry_submit($form, &$form_state) {
  db_set_active('legecy');
  if($form_state['values']['he_status']) {
      $allocation_insert = db_insert('Examiner_allocation_log') // Table name no longer needs {}
      ->fields(array(
        'paper_code' => $form_state['input']['allocation_papercode'],
        'Examiner_code' => $form_state['input']['head_examiner'],
        'total_sheets' => $form_state['input']['no_sheet_allocated'],
        'allocated_by' => $form_state['input']['allocated_by'],
        'allocation_date' => time(),
      ))
      ->execute();

  $limit = $form_state['values']['no_sheet_allocated'];
  $ans_update = "UPDATE answer_sheets SET status = :status, examiner = :head_examiner WHERE paper_code = :paper_code AND status = :check_status LIMIT $limit";
  $ans_updated = db_query($ans_update, array(
    ':status' => 'ALLOCATED',
    ':head_examiner' => $form_state['input']['head_examiner'],
    ':paper_code' => $form_state['input']['allocation_papercode'],
    ':check_status' => 'UNALLOCATED',
  ));
  }else{
    $allocation_insert = db_insert('allocation_log') // Table name no longer needs {}
      ->fields(array(
        'paper_code' => $form_state['input']['allocation_papercode'],
        'head_examiner' => $form_state['input']['head_examiner'],
        'center_code' => $form_state['input']['allocation_centercode'],
        'total_sheets' => $form_state['input']['no_sheet_allocated'],
        'allocated_by' => $form_state['input']['allocated_by'],
        'allocated_date' => time(),
      ))
      ->execute();

  $limit = $form_state['values']['no_sheet_allocated'];
  $ans_update = "UPDATE answer_sheets SET status = :status, head_examiner = :head_examiner WHERE paper_code = :paper_code AND status = :check_status LIMIT $limit";
  $ans_updated = db_query($ans_update, array(
    ':status' => 'ALLOCATED',
    ':head_examiner' => $form_state['input']['head_examiner'],
    ':paper_code' => $form_state['input']['allocation_papercode'],
    ':check_status' => 'UNALLOCATED',
  ));  
  }
  
  db_set_active('default'); // We need to call the main (drupal) db back  
  drupal_set_message(t('Answersheet allocated successfully.'));
  drupal_goto('manage/allocation');
}
