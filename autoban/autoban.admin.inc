<?php

// $Id: autoban.admin.inc,v 1.13 2008/10/23 07:18:26 ralph Exp $

// (C)2008 Pre-Emptive Limited. http://www.pre-emptive.net
// GNU Public Licenced (v2)
// Written by Ralph Bolton, September 2008

// autoban.admin.inc - Administrative functions

/**
 * Provide a form of settings for the module. This function is used to show the
 * main administration settings page.
 * @param none
 * @return an array of form items
 */
function autoban_admin_settings_form() {
  $form = array();

  $form['core'] = array(
    '#type' => 'fieldset',
    '#title' => t('Module Configuration'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );
  
  $form['core']['autoban_enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable Autoban'),
    '#description' => t('If enabled, Autoban will expire old Access Rules.'),
    '#default_value' => variable_get('autoban_enabled',0),
  );

  $form['core']['autoban_create_rules'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow Autoban to create Access Rules'),
    '#description' => t('If allowed, Autoban will automatically create Access Rules for IPs that exceed the thresholds below.'),
    '#default_value' => variable_get('autoban_create_rules',0),
  );

  $form['core']['autoban_incremental'] = array(
    '#type' => 'radios',
    '#title' => t('Access Log analysis'),
    '#description' => t('Incremental Access Log analysis checks the access log from the point the last check ended. Absolute checks the last "Time Threshold", regardless of when Autoban last looked at the access log.'),
    '#options' => array(0 => t('Absolute'), 1 => t('Incremental')),
    '#default_value' => variable_get('autoban_incremental',0),
  );

  $log_times = array(
    86400 => t('1 Day'),
    604800 => t('1 Week'),
    1209600 => t('2 Weeks'),
    2592000 => t('4 Weeks'),
    7776000 => t('3 Months'),
  );

  $form['core']['autoban_log_retention'] = array(
    '#type' => 'select',
    '#title' => t('Autoban Log Retention Time'),
    '#description' => t('The time before old entries are removed from the log file'),
    '#options' => $log_times,
    '#default_value' => variable_get('autoban_log_retention', 604800),
  );

  $options = array(
    60 => t('1 Minute'),
    900 => t('15 Minutes'),
    1800 => t('30 Minutes'),
    3600 => t('1 Hour'),
    21600 => t('6 Hours'),
    43200 => t('12 Hours'),
    86400 => t('1 Day'),
    604800 => t('1 Week'),
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

  $form['control'] = array(
    '#type' => 'fieldset',
    '#title' => t('Rules and Consequences'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  $form['control']['autoban_time_threshold'] = array(
    '#type' => 'select',
    '#title' => t('Transgression time period'),
    '#description' => t('The time Autoban watches to see if an IP address exceeds the hit count threshold below.'),
    '#options' => $options,
    '#default_value' => variable_get('autoban_time_threshold',3600),
  );

  $form['control']['autoban_max_hits_threshold'] = array(
    '#type' => 'textfield',
    '#title' => t('Hit count threshold'),
    '#size' => 8,
    '#description' => t('The number of hits an IP is allowed in the time limit above before being banned.'),
    '#default_value' => variable_get('autoban_max_hits_threshold',360),
  );

  $form['control']['autoban_ban_time'] = array(
    '#type' => 'select',
    '#title' => t('Ban Time'),
    '#description' => t('The time to ban IPs that exceed the thresholds above. Note: Bans are only removed when cron runs, so the minimum time a ban can stay in place is actually the time between cron runs.'),
    '#options' => $ban_times,
    '#default_value' => variable_get('autoban_ban_time', 3600),
  );

  $form['challenge'] = array(
    '#type' => 'fieldset',
    '#title' => t('Robot Detection Challenges'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $challenges = array(
    0 => t('None - No challenge issued'),
    1 => t('Plain HTML link'),
    2 => t('JavaScript link'),
    3 => t('Plain HTML image'),
    4 => t('JavaScript image'),
    5 => t('Simple mathematical question'),
    6 => t('ReCaptcha'),
  );
  $form['challenge']['autoban_tt_challenge'] = array(
    '#type' => 'select',
    '#title' => t('Challenge Type'),
    '#description' => t('The type of challenge (if any) used to detect humans and \'bots.'),
    '#options' => $challenges,
    '#default_value' => variable_get('autoban_tt_challenge', 0),
  );
  $form['challenge']['autoban_tt_ban_time'] = array(
    '#type' => 'select',
    '#title' => t('Fail Ban Time'),
    '#description' => t('The time to ban IPs that fail the challenge.'),
    '#options' => $ban_times,
    '#default_value' => variable_get('autoban_tt_ban_time', 3600),
  );
  $pass_times = array(0 => t('0 minutes')) + $ban_times;
  $form['challenge']['autoban_tt_pass_time'] = array(
    '#type' => 'select',
    '#title' => t('Pass Whitelist Time'),
    '#description' => t('The time to whitelist IPs that pass the challenge.'),
    '#options' => $pass_times,
    '#default_value' => variable_get('autoban_tt_pass_time', 900),
  );
  $form['challenge']['advanced'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['challenge']['advanced']['autoban_tt_displays'] = array(
    '#type' => 'textfield',
    '#title' => t('Challenge Displays'),
    '#description' => t('The number of times the challenge is displayed to the user.'),
    '#default_value' => variable_get('autoban_tt_displays', 10),
  );
  $form['challenge']['advanced']['autoban_tt_attempts'] = array(
    '#type' => 'textfield',
    '#title' => t('Challenge Attempts'),
    '#description' => t('The number of times the user can incorrectly respond to the challenge.'),
    '#default_value' => variable_get('autoban_tt_attempts', 3),
  );
  $form['challenge']['advanced']['autoban_tt_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Response Receiver Path'),
    '#description' => t('The Drupal path used to receive challenge responses.'),
    '#default_value' => variable_get('autoban_tt_path', '_autoban'),
  );
  $form['challenge']['advanced']['autoban_recaptcha_publickey'] = array(
    '#type' => 'textfield',
    '#title' => t('Recaptcha Public Key'),
    '#description' => t('Recaptcha public key'),
    '#default_value' => variable_get('autoban_recaptcha_publickey', ''),
  );
  $form['challenge']['advanced']['autoban_recaptcha_privatekey'] = array(
    '#type' => 'textfield',
    '#title' => t('Recaptcha private key'),
    '#description' => t('Recaptcha private key'),
    '#default_value' => variable_get('autoban_recaptcha_privatekey', ''),
  );
  $form['challenge']['advanced']['autoban_challenge_retention'] = array(
    '#type' => 'select',
    '#title' => t('Unused Challenge Retention'),
    '#description' => t('Challenges that are issued but not taken to conclusion are removed after this time.'),
    '#options' => $log_times,
    '#default_value' => variable_get('autoban_challenge_retention', 86400),
  );

  $form['subnet'] = array(
    '#type' => 'fieldset',
    '#title' => t('Subnet Detection'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['subnet']['autoban_subnet_detect_enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable Co-Ordinated Subnet Detection'),
    '#description' => t('If enabled, Autoban will identify and ban entire subnets if multiple hosts on it offend.'),
    '#default_value' => variable_get('autoban_subnet_detect_enabled',0),
  );
  $form['subnet']['autoban_subnet_size'] = array(
    '#type' => 'select',
    '#title' => t('Subnet Size'),
    '#description' => t('Defines how much address space Autoban considers to be a subnet.'),
    '#options' => array(1 => t('Class A - 255.0.0.0'), 2 => t('Class B - 255.255.0.0'), 3 => t('Class C - 255.255.255.0')),
    '#default_value' => variable_get('autoban_subnet_size',3),
  );
  $form['subnet']['autoban_subnet_offender_threshold'] = array(
    '#type' => 'textfield',
    '#title' => t('Subnet Offender Threshold'),
    '#size' => 8,
    '#description' => t('The number of offenders in a subnet before Autoban bans the entire subnet.'),
    '#default_value' => variable_get('autoban_subnet_offender_threshold', 10),
  );
  $form['subnet']['autoban_subnet_lookback_time'] = array(
    '#type' => 'select',
    '#title' => t('Subnet Detection Time Period'),
    '#description' => t('How far Autoban looks back in the log file for offenders during subnet detection. This cannot be longer than the log retention time.'),
    '#options' => $log_times,
    '#default_value' => variable_get('autoban_subnet_lookback_time', 604800),
  );
  $form['subnet']['autoban_subnet_ban_time'] = array(
    '#type' => 'select',
    '#title' => t('Subnet Ban Time'),
    '#description' => t('The time to ban a subnet if enough hosts on it offend. No challenge is offered when banning subnets.'),
    '#options' => $ban_times,
    '#default_value' => variable_get('autoban_subnet_ban_time', 86400),
  );
 
  $form['repeat'] = array(
    '#type' => 'fieldset',
    '#title' => t('Repeat Offenders'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['repeat']['autoban_repeat_enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable Repeat Offender Detection'),
    '#description' => t('If enabled, Autoban will identify and ban repeat offenders according to the settings below.'),
    '#default_value' => variable_get('autoban_repeat_enabled',0),
  );
  $form['repeat']['autoban_repeat_lookback_time'] = array(
    '#type' => 'select',
    '#title' => t('Repeat Offenders Time Period'),
    '#description' => t('How far Autoban looks back in the log file for repeat offenders. This cannot be longer than the log retention time.'),
    '#options' => $log_times,
    '#default_value' => variable_get('autoban_repeat_lookback_time', 604800),
  );
  $form['repeat']['autoban_repeat_offenses'] = array(
    '#type' => 'textfield',
    '#title' => t('Repeat Offenses Threshold'),
    '#size' => 8,
    '#description' => t('The number of offenses in the time limit above before being banned.'),
    '#default_value' => variable_get('autoban_repeat_offenses', 50),
  );
  $form['repeat']['autoban_repeat_ban_time'] = array(
    '#type' => 'select',
    '#title' => t('Repeat Offenses Ban Time'),
    '#description' => t('The time to ban IPs that repeatedly offend according to the rules above. No challenge is offered to repeat offenders.'),
    '#options' => $ban_times,
    '#default_value' => variable_get('autoban_repeat_ban_time', 86400),
  );

  if(module_exists('mollom')) {
    // If the Mollom module exists, show options relevent to it
    include_once('lib/autoban.mollom.inc');
    $form = array_merge($form, autoban_mollom_admin_form());
  }

  if(module_exists('beacon')) {
    // If the beacon module exists, show options relevent to it
    include_once('lib/autoban.beacon.inc');
    $form = array_merge($form, autoban_beacon_admin_form());
  }

  return system_settings_form($form);
}

// autoban_view_log()
// Returns an HTML fragment containing a table of recent Autoban activity.
// The data is taken from the autoban_log table and presented in a paged table.
function autoban_admin_view_log() {

  $header = array(
    array('data' => t('Timestamp'), 'field' => 'timestamp', 'sort' => 'desc'),
    array('data' => t('Active')),
    array('data' => t('Type'), 'field' => 'type'),
    array('data' => t('Mask'), 'field' => 'mask'),
    array('data' => t('Comments'), 'field' => 'notes'),
    array('data' => t('User'), 'field' => 'uid'),
  );

  if(variable_get('autoban_beacon_enabled', 0)) {
    $header = $header + array('data' => t('Beaconed'));
  }

  $sql = "SELECT * from {autoban_log}";
  $result = pager_query($sql . tablesort_sql($header), 25, 0, 'SELECT count(lid) FROM {autoban_log}');

  $rows = array();
  while($row = db_fetch_object($result)) {
    $cells = array();

    // Timestamp
    $cells[] = format_date($row->timestamp, 'custom', 'y-m-d H:i');

    // Active?
    if($row->reason == AUTOBAN_REASON_CHALLENGE_ISSUED) {
      //$active = autoban_is_challenged($row->mask);
      $active = db_result(db_query("SELECT 1 FROM {autoban_hitlist} WHERE hid=%d", $row->xid));
    } else {
      $active = db_result(db_query("SELECT 1 FROM {access} WHERE aid=%d", $row->xid));
    }
    if($active) {
      $cells[] = t('Yes');
    } else {
      $cells[] = '';
    }
    
    // Type
    $cells[] = t('@type', array('@type' => $row->type));

    // Mask
    if($row->mask != '' && module_exists('inetinfo')) {
      $cells[] = module_invoke('inetinfo','l',$row->mask);
    } else {
      $cells[] = $row->mask;
    }

    $text = '';

    if($row->reason == AUTOBAN_REASON_CHALLENGE_ISSUED) {
      $text .= 'Challenged';
    } else {
      if($row->status == 0) {
        $text .= 'Denied';
      } else {
        $text .= 'Allowed';
      }
      if($row->expires) {
        $diff = $row->expires - $row->timestamp;
        $text .= ' for ' . format_interval($diff);
        //$text .= ' until ' . format_date($row->expires, 'custom', 'y-m-d H:i');
      } else {
        $text .= ' forever';
      }
    }

    if($row->notes != '') {
      $text .= ' (' . $row->notes . ')';
    }
    
    $cells[] = $text;  


    // User
    if($row->uid) {
      $object = user_load($row->uid);
      $cells[] = theme('username', $object);
    } else {
      $cells[] = t('Autoban');
    }

    if(variable_get('autoban_beacon_enabled', 0)) {
      $score = module_invoke('beacon','get_score', $row->mask);
      if(is_null($score)) {
        $cells[] = t('Yes');
      } elseif($score == FALSE) {
        $cells[] = '';
      } else {
        $cells[] = $score;
      }
    }

    $rows[] = $cells;
  }
  $content = '';
  $content .= '<div>' . l(t('View Access Rules'), 'admin/user/rules') . '</div>';

  $content .= theme('table', $header, $rows);
  $content .= theme('pager', NULL, 25, 0);
  return $content;
}

// autoban_admin_view_current_activity()
// Returns an HTML fragment containing a table of current users of the site.
// It gets this information from the accesslog table, but adds in Autoban
// information (such as the status of that visitor, their authorisation state
// etc).
function autoban_admin_view_current_activity() {

  // Now work out how many hits an IP must have made since our last cron run
  // to be over the threshold...
  // If we're in incremental mode, then we work this out based on the last
  // Cron run time (otherwise we just use the last $time_threshold minutes of
  // accesslog). For Incremental, we have to multiply up the hits threshold
  // to match the time difference since the last cron run
  $time_now = time();
  $time_threshold = variable_get('autoban_time_threshold', 3600);
  $max_hits_threshold = variable_get('autoban_max_hits_threshold', 360);
  if(variable_get('autoban_incremental', 0)) {
    // Get the last cron run time, or 15 minutes ago (if cron never run)
    $last_cron = variable_get('cron_last', $time_now - 15*60);
    $time_diff = $time_now - $last_cron;
    $divider = $time_threshold / $time_diff;
    $point_in_time = $last_cron;
    // Now divvy up the hits as required...
    $hits_required = $max_hits_threshold / $divider;
  } else {
    // Not incremental, so just look back $time_threshold seconds...
    $point_in_time = $time_now - $time_threshold;
    $hits_required = $max_hits_threshold;
  } 

  $header = array(
    array('data' => t('IP Address'), 'field' => 'hostname'),
    array('data' => t('Page Views'), 'field' => 'hits', 'sort' => 'desc'),
    array('data' => t('Last Seen'), 'field' => 'timestamp'),
    array('data' => t('Status')),
    array('data' => t('Authorisation')),
  );
  if(variable_get('autoban_beacon_enabled', 0)) {
    $header = $header + array('data' => t('Beaconed'));
  }

  $sql = "SELECT COUNT(uid) AS hits, hostname, MAX(timestamp) as timestamp FROM {accesslog} WHERE timestamp>%d GROUP BY hostname";

  $result = pager_query($sql . tablesort_sql($header), 25, 0, "SELECT COUNT(DISTINCT(hostname)) FROM {accesslog} WHERE timestamp>%d", $point_in_time);

  $rows = array();
  while($row = db_fetch_object($result)) {
    $cells = array();
    if(module_exists('inetinfo')) {
      $cells[] = module_invoke('inetinfo','l',$row->hostname);
    } else {
      $cells[] = $row->hostname;
    }
    $cells[] = $row->hits;
    $cells[] = format_date($row->timestamp, 'short');
    // Now work out if it's over or under threshold
    if($row->hits > $hits_required) {
      $cells[] = t('Over Limit');
    } else {
      $cells[] = '';
    }

    // Now work out if it's authorised or not...
    $state = autoban_is_denied('host', $row->hostname);
    if(is_null($state)) {
      // If it's neither banned nor whitelisted, it might be on the hitlist
      if(autoban_is_challenged($row->hostname)) {
        $cells[] = t('Challenged');
      } else {
        $cells[] = '';
      }
    } elseif($state == TRUE) {
      $cells[] = t('Banned');
    } else {
      $cells[] = t('Whitelisted');
    }
    if(variable_get('autoban_beacon_enabled', 0)) {
      $score = module_invoke('beacon','get_score', $row->hostname);
      if(is_null($score)) {
        $cells[] = t('Yes');
      } elseif($score == FALSE) {
        $cells[] = '';
      } else {
        $cells[] = $score;
      }
    }
    $rows[] = $cells;
  }

  $content = t('Listing top visitors between @start and @end.', array('@start' => format_date($point_in_time, 'short'), '@end' => format_date($time_now, 'short')));

  $content .= theme('table', $header, $rows);
  $content .= theme('pager', NULL, 25, 0);
  return $content;
}

// autoban_admin_view_current_challenges()
// Returns an HTML fragment containing a table of the current (outstanding)
// challenges to users. The information comes from the autoban_hitlist table.
// Generally, this table should be pretty small ;-)
function autoban_admin_view_current_challenges() {
  $header = array(
    array('data' => t('IP Address'), 'field' => 'mask', 'sort' => 'asc'),
    array('data' => t('Remaining'), 'field' => 'remaining'),
    array('data' => t('Tries Left'), 'field' => 'triesleft'),
    array('data' => t('Last Change'), 'field' => 'timestamp'),
  );

  $sql = "SELECT * FROM {autoban_hitlist}";

  $result = pager_query($sql . tablesort_sql($header), 25, 0, "SELECT COUNT(hid) FROM {autoban_hitlist}");

  $rows = array();
  while($row = db_fetch_object($result)) {
    $cells = array();
    if(module_exists('inetinfo')) {
      $cells[] = module_invoke('inetinfo','l',$row->mask);
    } else {
      $cells[] = $row->mask;
    }
    $cells[] = $row->remaining;
    $cells[] = $row->triesleft;
    $cells[] = format_date($row->timestamp, 'short');
    $rows[] = $cells;
  }
  $content = '';
  $content .= t('Listing currently open challenges to visitors.');
  $content .= theme('table', $header, $rows);
  $content .= theme('pager', NULL, 25, 0);
  return $content;
}

// The following line is for Vim users - please don't delete it.
// vim: set filetype=php expandtab tabstop=2 shiftwidth=2:
