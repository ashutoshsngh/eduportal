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
  $form['total_allocation_label'] = array(
    '#type' => 'item',
    '#markup' => 'Total Unallocated : ' . answersheet_unallocated_count($code),
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
  $form['head_examiner'] = array(
    '#type' => 'select',
    "#title" => t("Select Head Examiner"),
    '#options' => exam_portal_all_head_examiner(),
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Allocation'),
  );
  return $form;
}

/**
 * Submit handler portal_entry_add_question_settings().
 */
function allocation_details_entry_submit($form, &$form_state) {
  db_set_active('legecy');
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
  db_set_active('default'); // We need to call the main (drupal) db back  
  drupal_set_message(t('Answersheet allocated successfully.'));
  drupal_goto('manage/allocation');
}
