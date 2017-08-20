<?php

// $Id: autoban.mollom.inc,v 1.3 2008/10/24 11:56:51 ralph Exp $

// (C)2008 Pre-Emptive Limited. http://www.pre-emptive.net
// GNU Public Licenced (v2)
// Written by Ralph Bolton, September 2008

// This library provides integration with the Mollom anti-spam module. It is only used if the Mollom
// module is installed.

// Cron hook (called by autoban_cron())
function autoban_mollom_cron() {

  // Do nothing if we're not specifically enabled
  if(!variable_get('autoban_mollom_enabled', 0)) {
    return '';
  }

  $look_back = variable_get('autoban_mollom_lookback_time', 604800);
  $point_in_time = time() - $look_back;

  $result=db_query("SELECT hostname FROM {watchdog} WHERE timestamp>%d AND message='Spam: %message' OR message='Incorrect CAPTCHA'", $point_in_time);

  // Get the threshold...
  $mollom_threshold = variable_get('autoban_mollom_events', 10);

  // And work out the expiry time...
  $ban_time = variable_get('autoban_mollom_ban_time', 86400);
  if($ban_time) {
    $expires = time() + $ban_time;
  } else {
    $expires = 0;
  }

  $ips = array();
  while($row = db_fetch_object($result)) {
    
    $state = autoban_is_denied('host', $row->hostname);
    if(!is_null($state)) {
      // Banned or whitelisted already - skip it
      continue;
    }

    if(isset($ips[$row->hostname])) {
      $ips[$row->hostname]++;
    } else {
      $ips[$row->hostname] = 1;
    }
    if($ips[$row->hostname] > $mollom_threshold) {
      // This IP has exceeded the threshold, and is not black/whitelisted
      // so we can add a rule for this one...
      $notes = $ips[$row->hostname] . ' Mollom events exceeds limit of ' . $mollom_threshold . ' in ' . format_interval($look_back);
      autoban_add_rule(array('expires' => $expires, 'mask' => $row->hostname, 'notes' => $notes, 'reason' => AUTOBAN_REASON_MOLLOM));
    }
  }

  return '';
}

// Admin settings form hook
function autoban_mollom_admin_form() {
  $form['mollom'] = array(
    '#type' => 'fieldset',
    '#title' => t('Mollom Anti-Spam Module Integration'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['mollom']['autoban_mollom_enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable Autoban/Mollom Integration'),
    '#description' => t('If enabled, Autoban will use Mollom events as a basis for access rule creation.'),
    '#default_value' => variable_get('autoban_mollom_enabled',0),
  );

  $form['mollom']['autoban_mollom_events'] = array(
    '#type' => 'textfield',
    '#title' => t('Beacon Events Threshold'),
    '#size' => 10,
    '#description' => t('An IP having more than this number of events in the Watchdog log will be banned.'),
    '#default_value' => variable_get('autoban_mollom_events',10),
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

  $form['mollom']['autoban_mollom_ban_time'] = array(
    '#type' => 'select',
    '#title' => t('Ban Time'),
    '#description' => t('The time to ban IPs that exceed the thresholds above. Note: Bans are only removed when cron runs, so the minimum time a ban can stay in place is actually the time between cron runs.'),
    '#options' => $ban_times,
    '#default_value' => variable_get('autoban_mollom_ban_time', 86400),
  );

  $log_times = array(
    86400 => t('1 Day'),
    604800 => t('1 Week'),
    1209600 => t('2 Weeks'),
    2592000 => t('4 Weeks'),
    7776000 => t('3 Months'),
  );

  $form['mollom']['autoban_mollom_lookback_time'] = array(
    '#type' => 'select',
    '#title' => t('Mollom Time Period'),
    '#description' => t('How far Autoban looks back in the log file for Mollom events. This cannot be longer than the watchdog log retention time.'),
    '#options' => $log_times,
    '#default_value' => variable_get('autoban_mollom_lookback_time', 604800),
  );

  return $form;
}

// The following line is for Vim users - please don't delete it.
// vim: set filetype=php expandtab tabstop=2 shiftwidth=2:
