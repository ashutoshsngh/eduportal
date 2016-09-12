<?php

/**
 * @file
 * Manage allocation details.
 */

/**
 * Mangage allocation callback.
 */
function manage_allocation_callback() {
  global $user;
  $keys = array_keys($user->roles);
  $loggedinuser = get_userdata($user->mail);
  db_set_active('legecy');
  if (in_array(EXTERNAL_HEAD_EXAMINER_RID, $keys)) {
    $id = $loggedinuser['id'];
    $query = db_query("SELECT paper_code,STATUS , count( * ) AS 'count' FROM answer_sheets where head_examiner = $id GROUP BY paper_code,STATUS");
  }
  else {
    $query = db_query("SELECT paper_code,STATUS , count( * ) AS 'count' FROM answer_sheets GROUP BY paper_code,STATUS");
  }
  $result = $query->fetchAll();
  db_set_active('default');
  $header = array(
    array('data' => t('Paper Code')),
    array('data' => t('Total sheets')),
    array('data' => t('Allocated')),
    array('data' => t('Unallocated Sheets')),
    array('data' => t('Action')),
  );
  $rows = array();
  if (!empty($result)) {
    foreach ($result as $value) {
      if ($value->STATUS == 'ALLOCATED') {
        $key = 'allocated';
      }
      elseif($value->STATUS == 'CHECKED') {
        $key = 'checked';
      }
      else {
        $key = 'unallocated';
      }
      $rows[$value->paper_code][$key] = $value->count;
    }
    $data = array();
    foreach ($rows as $key => $value) {
      if (array_key_exists('allocated', $value) && array_key_exists('unallocated', $value)) {
        $total = $value['allocated'] + $value['unallocated'];
      }
      elseif (array_key_exists('allocated', $value) && !array_key_exists('unallocated', $value)) {
        $total = $value['allocated'];
      }
      else {
        $total = $value['unallocated'];
      }
      
      $data[] = array(
        $key,
        $total,
        isset($value['allocated']) ? $value['allocated'] : 0,
        isset($value['unallocated']) ? $value['unallocated'] : 0,
        l(t('Allocate'), 'allocation_entry/' . $key),
      );
    }
  }
    $build = array();
    $build['table'] = array(
      '#markup' => theme('table', array('header' => $header, 'rows' => $data,'empty' => 'no')),
    );
    $build['pager'] = array(
      '#markup' => theme('pager'),
    );
    return $build;
}
