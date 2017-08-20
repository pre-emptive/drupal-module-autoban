<?php

// $Id: autoban.beacon.inc,v 1.6 2008/10/21 15:01:28 ralph Exp $

// (C)2008 Pre-Emptive Limited. http://www.pre-emptive.net
// GNU Public Licenced (v2)
// Written by Ralph Bolton, September 2008

// This library provides integration with the Beacon module. It is only used if the Beacon
// module is installed.

// Cron hook (called by autoban_cron())
function autoban_beacon_cron() {

  // Do nothing if we're not specifically enabled
  if(!variable_get('autoban_beacon_enabled', 0)) {
    return '';
  }

  // Get the autoban beacon score threshold
  $beacon_score_threshold = variable_get('autoban_beacon_score_threshold', 10000);

  // Now grab any beacon IPs above that score...
  $result=db_query("SELECT score,ipaddress FROM {beacon_state} WHERE score>=%d ORDER BY score DESC", $beacon_score_threshold);

  $ban_time = variable_get('autoban_beacon_ban_time', 1209600);
  if($ban_time) {
    $expires = time() + $ban_time;
  } else {
    $expires = 0;
  }

  while($row = db_fetch_object($result)) {
    $state = autoban_is_denied('host', $row->ipaddress);
    if(is_null($state)) {
      $x = '';
    } elseif($state) {
      $x = '*';
    } else {
      $x = '+';
    }
    if(is_null($state)) {
      $notes = 'Beacon score ' . $row->score . " exceeds $beacon_score_threshold";
      autoban_add_rule(array('expires' => $expires, 'mask' => $row->ipaddress, 'notes' => $notes, 'reason' => AUTOBAN_REASON_BEACON));
    }
  }

  return '';
}

// Admin settings form hook
function autoban_beacon_admin_form() {
  $form['beacon'] = array(
    '#type' => 'fieldset',
    '#title' => t('Beacon Module Integration'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['beacon']['autoban_beacon_enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable Autoban/Beacon Integration'),
    '#description' => t('If enabled, Autoban will use Beacon module information as a basis for access rule creation.'),
    '#default_value' => variable_get('autoban_beacon_enabled',0),
  );

  $form['beacon']['autoban_beacon_score_threshold'] = array(
    '#type' => 'textfield',
    '#title' => t('Beacon Score Threshold'),
    '#size' => 10,
    '#description' => t('Beaconed IPs above this will be banned'),
    '#default_value' => variable_get('autoban_beacon_score_threshold',10000),
  );

  $ban_times = array(
    300 => t('5 Minutes'),
    900 => t('15 Minutes'),
    1800 => t('30 Minutes'),
    3600 => t('1 Hour'),
    21600 => t('6 Hours'),
    43200 => t('12 Hours'),
    86400 => t('1 Day'),
    604800 => t('7 days'),
    1209600 => t('14 days'),
    2592000 => t('30 days'),
    7776000 => t('90 days'),
    0 => t('For ever!'),
  );

  $form['beacon']['autoban_beacon_ban_time'] = array(
    '#type' => 'select',
    '#title' => t('Ban Time'),
    '#description' => t('The time to ban IPs that exceed the thresholds above. Note: Bans are only removed when cron runs, so the minimum time a ban can stay in place is actually the time betweencron runs.'),
    '#options' => $ban_times,
    '#default_value' => variable_get('autoban_beacon_ban_time', 1209600),
  );

  return $form;
}

// The following line is for Vim users - please don't delete it.
// vim: set filetype=php expandtab tabstop=2 shiftwidth=2:
