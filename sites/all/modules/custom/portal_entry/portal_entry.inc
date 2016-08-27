<?php

/**
 * @file
 * Inc file for contact us admin settings.
 */

/**
 * Callback function for contact us settings form.
 */
function portal_entry_add_question_settings($form_state) {
  $form = array();
  $form['pe_exam_code'] = array(
    '#type' => 'select',  
    "#title" => t("Exam Code"),
    '#options' => exam_portal_all_exam_code(),
  );
  $form['pe_subject_code'] = array(
    '#type' => 'select',  
    "#title" => t("Subject Code"),
    '#options' => exam_portal_all_subject_code(),      
  );
  $form['pe_paper_code'] = array(
    "#type" => 'textfield',
    "#title" => t("Paper Code"),
  );
  $form['pe_region'] = array(
    '#type' => 'select',  
    "#title" => t("Region Code"),
    '#options' => exam_portal_all_region_code(),      
  );
  $form['pe_language'] = array(
    '#type' => 'select',  
    "#title" => t("Language/Medium"),
    '#options' => array(
      'hindi' => t('Hindi'),
      'english' => t('English'),
    ),
  );
  $form['pe_paper_set'] = array(
    '#type' => 'select',  
    "#title" => t("Paper Set"),
    '#options' => array(
      'a' => t('A'),
      'b' => t('B'),
      'c' => t('C'),  
    ),
  );

  $form['pe_main_ans_sheet_count'] = array(
    '#type' => 'textfield',
    '#title' => t('Sheet Count'),
  );  
  
  $form['pe_supp_ans_sheet_count'] = array(
    '#type' => 'textfield',
    '#title' => t('Sheet Count'),
  );    
  
  $form['pe_model_ques_file'] = array(
    "#type" => 'textfield',
    "#title" => t("Model question file"),
  );  
  $form['pe_model_ans_file'] = array(
    "#type" => 'textfield',
    "#title" => t("Model answer file"),
  );    
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}

/**
 * Submit handler portal_entry_add_question_settings().
 */
function portal_entry_add_question_settings_submit($form, &$form_state) {
    db_set_active('legecy');
    $question_insert = db_insert('question_papers') // Table name no longer needs {}
    ->fields(array(
      'exam_id' => $form_state['input']['pe_exam_code'],
      'subject_id' => $form_state['input']['pe_subject_code'],
      'region_id' => $form_state['input']['pe_region'],
      'paper_code' => $form_state['input']['pe_paper_code'],
      'subject_code' => '',
      'language' => $form_state['input']['pe_language'],
      'paper_set' => $form_state['input']['pe_paper_set'],
      'model_question_link' => $form_state['input']['pe_model_ques_file'],
      'model_answer_link' => $form_state['input']['pe_model_ans_file'],  
      'main_sheet_page_count' => $form_state['input']['pe_main_ans_sheet_count'],
      'suppl_sheet_page_count' => $form_state['input']['pe_supp_ans_sheet_count'],
    ))
    ->execute();    
    db_set_active('default'); // We need to call the main (drupal) db back  
    drupal_set_message(t('inserted successfully.'));  
}

/**
 * function to show Edit.
 */
function portal_entry_add_question_edit($form_state) {
    $edit_id = arg(2);
    db_set_active('legecy');    
    $query = db_select('question_papers', 'qp');
    $query->condition('id', $edit_id, '=');
    $query->fields('qp');
    $result = $query->execute()->fetchAssoc();
       // $result = $query->fetchAll();
    db_set_active('default'); // We need to call the main (drupal) db back      
    
  $form = array();
  $form['pe_exam_code'] = array(
    '#type' => 'select',  
    "#title" => t("Exam Code"),
    '#options' => exam_portal_all_exam_code(),
    '#default_value' => $result['paper_code'],  
  );
  $form['pe_subject_code'] = array(
    '#type' => 'select',  
    "#title" => t("Subject Code"),
    '#options' => exam_portal_all_subject_code(),      
    '#default_value' => $result['subject_id'],  
  );
  $form['pe_paper_code'] = array(
    "#type" => 'textfield',
    "#title" => t("Paper Code"),
    '#default_value' => $result['paper_code'],  
  );
  $form['pe_region'] = array(
    '#type' => 'select',  
    "#title" => t("Region Code"),
    '#options' => exam_portal_all_region_code(),      
    '#default_value' => $result['region_id'],  
  );
  $form['pe_language'] = array(
    '#type' => 'select',  
    "#title" => t("Language/Medium"),
    '#options' => array(
      'hindi' => t('Hindi'),
      'english' => t('English'),
    ),
    '#default_value' => $result['language'],  
  );
  $form['pe_paper_set'] = array(
    '#type' => 'select',  
    "#title" => t("Paper Set"),
    '#options' => array(
      'a' => t('A'),
      'b' => t('B'),
      'c' => t('C'),  
    ),
    '#default_value' => $result['paper_set'],  
  );

  $form['pe_main_ans_sheet_count'] = array(
    '#type' => 'textfield',
    '#title' => t('Sheet Count'),
    '#default_value' => $result['main_sheet_page_count'],  
  );  
  
  $form['pe_supp_ans_sheet_count'] = array(
    '#type' => 'textfield',
    '#title' => t('Sheet Count'),
    '#default_value' => $result['suppl_sheet_page_count'],  
  );    
  
  $form['pe_model_ques_file'] = array(
    "#type" => 'textfield',
    "#title" => t("Model question file"),
    '#default_value' => $result['model_question_link'],  
  );  
  $form['pe_model_ans_file'] = array(
    "#type" => 'textfield',
    "#title" => t("Model answer file"),
    '#default_value' => $result['model_answer_link'],  
  );    
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}

/**
 * Submit handler portal_entry_edit().
 */
function portal_entry_add_question_edit_submit($form, &$form_state) {
    $edit_id = arg(2);
    db_set_active('legecy');
    $form_state['redirect'] = 'added_question_listing';
    $question_insert = db_update('question_papers') // Table name no longer needs {}
    ->fields(array(
      'exam_id' => $form_state['input']['pe_exam_code'],
      'subject_id' => $form_state['input']['pe_subject_code'],
      'region_id' => $form_state['input']['pe_region'],
      'paper_code' => $form_state['input']['pe_paper_code'],
      'subject_code' => '',
      'language' => $form_state['input']['pe_language'],
      'paper_set' => $form_state['input']['pe_paper_set'],
      'model_question_link' => $form_state['input']['pe_model_ques_file'],
      'model_answer_link' => $form_state['input']['pe_model_ans_file'],  
      'main_sheet_page_count' => $form_state['input']['pe_main_ans_sheet_count'],
      'suppl_sheet_page_count' => $form_state['input']['pe_supp_ans_sheet_count'],
    ))
  ->condition('id', $edit_id, '=')
  ->execute();        
    db_set_active('default'); // We need to call the main (drupal) db back  
    
    drupal_set_message(t('updated successfully.'));  
}

/**
 * function to show all listing.
 */
function portal_entry_added_question_listing() {
  $header = array(
    array('data' => t('Exam Code')),
    array('data' => t('Region Code')),
    array('data' => t('Language')),
    array('data' => t('Set')),  
    array('data' => t('Edit')),
      );  
    db_set_active('legecy');    
    $query = db_select('question_papers', 'qp')
        ->fields('qp', array('paper_code','region_id', 'language', 'paper_set', 'id'));
    
    $result = $query->execute();
    db_set_active('default'); // We need to call the main (drupal) db back      
    
  $rows = array();
  foreach ($result as $row) {
    $rows[] = array('data' => array(
                            'paper_code' => $row->paper_code,
                            'region_id' => $row->region_id,
                            'language' => $row->language,
                            'paper_set' => $row->paper_set,
                            'id' => l(t('Edit'), 'add-question-data/edit/'.$row->id),
                        ),        
            );
  }
  // Build the table for the nice output.
  $build['question_table_listing'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
  );

  return $build;
}