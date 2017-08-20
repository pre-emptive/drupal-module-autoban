<?php

// $Id: autoban.access.inc,v 1.6 2008/10/21 10:19:19 ralph Exp $

// (C)2008 Pre-Emptive Limited. http://www.pre-emptive.net
// GNU Public Licenced (v2)
// Written by Ralph Bolton, September 2008

// autoban.access.inc - Overrides for the Drupal Core Access features
// This library provides all of the functions that 'override' the core User
// Access features. We override these in hook_menu(), and the implementations
// are here. For the most part, the functions here are cut-and-pasted from
// the user.module code, although in some cases we're able to simplify a
// little. Of course, the Autoban extras are added as required.

// autoban_admin_user_rules_form()
// form_state - provided by Drupal
// edit - an array of existing values (perhaps fromt he DB)
// submit - the name of the 'submit' button on the page
// handler - not used!??
// This function displays the main add/edit rule screen. It allows the user to
// add deny or allow rules, for users, email or hostname patterns, allows the
// rule to have an expiry date, and an optional comment (as an aide-memoire)
function autoban_admin_user_rules_form(&$form_state, $edit = array(), $submit = NULL, $handler = NULL) {
  $form['aid'] = array(
    '#type' => 'hidden',
    '#value' => $edit['aid'],
  );
  $form['status'] = array(
    '#type' => 'radios',
    '#title' => t('Access type'),
    '#default_value' => isset($edit['status']) ? $edit['status'] : 0,
    '#options' => array('1' => t('Allow'), '0' => t('Deny')),
  );
  $type_options = array('user' => t('Username'), 'mail' => t('E-mail'), 'host' => t('Host'));
  $form['type'] = array(
    '#type' => 'radios',
    '#title' => t('Rule type'),
    '#default_value' => (isset($type_options[$edit['type']]) ? $edit['type'] : 'user'),
    '#options' => $type_options,
  );
  $form['mask'] = array(
    '#type' => 'textfield',
    '#title' => t('Mask'),
    '#size' => 30,
    '#maxlength' => 64,
    '#default_value' => $edit['mask'],
    '#description' => '%: '. t('Matches any number of characters, even zero characters') .'.<br />_: '. t('Matches exactly one character.'),
    '#required' => TRUE,
  );

  $form['autoban_ban_time'] = array(
    '#type' => 'select',
    '#title' => t('Rule Time'),
    '#default_value' => 2592000,
    '#options' => array(
      86400 => t('1 day'),
      604800 => t('7 days'),
      1209600 => t('14 days'),
      2592000 => t('30 days'),
      7776000 => t('90 days'),
      0 => t('Forever!'),
    ),
    '#description' => t('The amount of time that this rule will exist'),
  );

  $form['autoban_notes'] = array(
    '#type' => 'textfield',
    '#title' => t('Notes'),
    '#size' => 50,
    '#maxlength' => 250,
    '#default_value' => $edit['notes'],
    '#description' => t('Optional notes about this rule.'),
  );

  if(!isset($submit)) {
    $submit = t('Add Rule');
  }

  $form['submit'] = array('#type' => 'submit', '#value' => $submit);

  return $form;
}

// autoban_list_rules()
// Returns an HTML fragment which contains a table of the rules currently configured
// on the system.
function autoban_list_rules() {

  $header = array(
    array('data' => t('Mask'), 'field' => 'mask', 'sort' => 'asc'),
    array('data' => t('Type'), 'field' => 'type'),
    array('data' => t('Action'), 'field' => 'status'),
    array('data' => t('Expires'), 'field' => 'expires'),
    array('data' => t('Created'), 'field' => 'created'),
    array('data' => t('User'), 'field' => 'uid'),
    array('data' => t('Operations'), 'colspan' => 2),
  );

  $sql = "SELECT a.*,t.created,t.expires,t.uid,t.notes FROM {access} AS a LEFT JOIN {autoban_targets} AS t ON a.aid = t.aid";

  // Now step through the autoban_targets table and list out what's in it
  $result = pager_query($sql . tablesort_sql($header), 25, 0, "SELECT count(aid) FROM {access}");
  $rows = array();
  $userload = NULL;
  while($row = db_fetch_object($result)) {
    $cells = array();
    $attributes = array('title' => $row->notes);
    // If the inetinfo module exists, use it for full IP addresses only
    if(module_exists('inetinfo') && $row->type == 'host' && !strpos($row->mask, '%') && !strpos($row->mask, '_')) {
      $cells[] = module_invoke('inetinfo','l',$row->mask, '', array('attributes' => $attributes));
    } else {
      $cells[] = l($row->mask, $_REQUEST['q'], array('attributes' => $attributes));
    }
    $cells[] = t($row->type);
    $cells[] = $row->status ? t('Allow') : t('Deny');
    $cells[] = $row->expires ? format_date($row->expires, 'short') : t('Never');
    $cells[] = $row->created ? format_date($row->created, 'short') : '-';
    if(is_null($row->uid)) {
      $username = '-';
    } elseif($row->uid) {
      if($row->uid != $userload->uid) {
        $userload = user_load($row->uid);
      }
      $username = theme('username', $userload);
    } else {
      $username = t('Autoban');
    }
    $cells[] = $username;
    $cells[] = l(t('Edit'), 'admin/user/rules/edit/' . $row->aid);
    $cells[] = l(t('Delete'), 'admin/user/rules/delete/' . $row->aid);
    $rows[] = $cells;
  }

  $output = '';
  $output .= '<div>' . l(t('View Autoban Log'), 'admin/reports/autoban') . '</div>';

  $output .= theme('table', $header, $rows);
  $output .= theme('pager', NULL, 25, 0);

  return $output;
}

// autoban_admin_user_rules_form_submit()
// Submit handler for rule add/edit. Called by Drupal Forms system.
function autoban_admin_user_rules_form_submit($form, &$form_state) {
  if($form_state['values']['aid']) {
    // This is an edit, so remove the old rule and insert a fresh one.
    // This is a fairly cheesy way to do edits, but it's easy ;-)
    db_query("DELETE FROM {access} WHERE aid=%d", $form_state['values']['aid']);
    db_query("DELETE FROM {autoban_targets} WHERE aid=%d", $form_state['values']['aid']);
  }

  $now = time();
  if($form_state['values']['autoban_ban_time']) {
    $then = $now + $form_state['values']['autoban_ban_time'];
  } else {
    $then = 0;
  }

  autoban_add_rule(array( 'expires' => $then,
                          'mask' => $form_state['values']['mask'],
                          'type' => $form_state['values']['type'],
                          'reason' => AUTOBAN_REASON_IGNORED_CHALLENGE,
                          'notes' => $form_state['values']['autoban_notes'],
                          'status' => $form_state['values']['status']));

  drupal_set_message(t('Rule for ' . $form_state['values']['type'] . ' ' . $form_state['values']['mask'] . ' has been added'));
}

// autoban_user_rules_edit()
// Wrapper to allow for editing of rules. Reads the current information from
// the database and then tell Drupal to call the normal user_rules_form, with
// that information in the $edit parameter.
function autoban_user_rules_edit($aid = 0) {
  $edit = db_fetch_array(db_query('SELECT a.aid, a.type, a.status, a.mask, t.notes FROM {access} AS a LEFT JOIN {autoban_targets} AS t ON a.aid=t.aid WHERE a.aid = %d', $aid));
  return drupal_get_form('autoban_admin_user_rules_form', $edit, t('Save rule'));
}

// The following line is for Vim users - please don't delete it.
// vim: set filetype=php expandtab tabstop=2 shiftwidth=2:
