<?php

// $Id: autoban.cron.inc,v 1.9 2008/11/05 13:12:15 ralph Exp $

// (C)2008 Pre-Emptive Limited. http://www.pre-emptive.net
// GNU Public Licenced (v2)
// Written by Ralph Bolton, September 2008

function _microtime_float() {
  list($usec,$sec) = explode(' ', microtime());
  return ((float)$usec + (float)$sec);
}

/**
 * Actual implementation of hook_cron()
 * This function is included and run as required
 * @param none
 * @return none
 */
function _autoban_cron($delete_rules = 0, $create_rules = 0) {

  $content = '';

  // When is now?
  $time_now = time();

  // First, clean out autoban log entries
  $result = db_query('DELETE FROM {autoban_log} WHERE timestamp < %d', ($time_now - variable_get('autoban_log_retention', 604800)));

  // Clean out old challenges
  $result = db_query('DELETE FROM {autoban_hitlist} WHERE timestamp < %d', ($time_now - variable_get('autoban_challenge_retention', 86400)));

  // We start by removing old rules...
  if($delete_rules) {

    // Fetch all AIDs for all bans that have expired
    $result = db_query('SELECT aid FROM {autoban_targets} WHERE expires < %d AND expires != 0', $time_now);

    // Delete all {access} entries and then then then any orphaned autoban targets
    while($row = db_fetch_object($result)) {
        db_query('DELETE FROM {access} WHERE aid=%d', $row->aid);
    }

    // Now delete any orphaned entries in the autoban_targets table
    $result = db_query('SELECT aid FROM {autoban_targets}');
    while($row = db_fetch_object($result)) {
      if(!db_result(db_query('SELECT aid FROM {access} WHERE aid=%d', $row->aid))) {
        db_query('DELETE FROM {autoban_targets} WHERE aid=%d', $row->aid);
      } 
    }
  } // end of if delete rules

  // Now we get onto adding new rules...

  if($create_rules) {

    // Do subnet detection
    if(variable_get('autoban_subnet_detect_enabled',0)) {
      // Get a list of all infractions in the lookback time
      // Don't get any wildcards, or repeat rules
      $look_back = variable_get('autoban_repeat_lookback_time', 604800);
      $point_in_time = $time_now - $look_back;
      $result = db_query("SELECT DISTINCT(mask) FROM {autoban_log} WHERE timestamp>%d AND type='host' AND reason>128 and reason!=%d AND mask NOT LIKE '%\%%' AND mask NOT LIKE '%\_%'", $point_in_time, AUTOBAN_REASON_REPEAT);

      // Now get subnet size required
      $subnet_size = variable_get('autoban_subnet_size',3);

      // And the threshold
      $threshold = variable_get('autoban_subnet_offender_threshold', 10);

      $subnet_members = array();
      while($row = db_fetch_object($result)) {
        // First, trim the mask down to subnet size...
        $parts = explode('.', $row->mask);
        $subnet = implode('.', array_slice($parts,0,$subnet_size));
        //error_log("Subnet Detect: IP=" . $row->mask . " subnet=$subnet");
        if(isset($subnet_members[$subnet])) {
          $subnet_members[$subnet]++;
          // now see if this subnet is a problem because too many of it's members have offended
          if($subnet_members[$subnet] > $threshold) {
            // Ban the subnet because it's got too many violators on it, if it doesn't already exist
            $state = autoban_is_denied('host', $subnet . '.%');
            if(!is_null($state)) {
              continue;
            }
            $notes = $subnet_members[$subnet] . " bans on subnet in " . format_interval($look_back);
            $expires = $time_now + variable_get('autoban_subnet_ban_time', 86400);
            autoban_add_rule(array('expires' => $expires, 'mask' => $subnet . '.%', 'notes' => $notes, 'reason' => AUTOBAN_REASON_SUBNET));
          }
        } else {
          $subnet_members[$subnet] = 1;
        }
      }
      
    }

    // Do repeat offender detection
    if(variable_get('autoban_repeat_enabled',0)) {
      // Get a list of the top repeat offenders from the log, only from whenever in the past
      // has been configured in the settings
      $look_back = variable_get('autoban_repeat_lookback_time', 604800);
      $point_in_time = $time_now - $look_back;
      $result = db_query("SELECT COUNT(mask) as rules, mask FROM {autoban_log} WHERE timestamp > %d AND reason>128 AND mask!='' GROUP BY mask ORDER BY rules DESC", $point_in_time);
      $max_offenses = variable_get('autoban_repeat_offenses', 50);
      $expires = $time_now + variable_get('autoban_repeat_ban_time', 86400);
      while($row = db_fetch_object($result)) {
        if($row->rules < $max_offenses) {
          // Too few offenses to worry about
          break;
        }
        // Okay - this is an offender we can ban
        $state = autoban_is_denied('host', $row->mask);
        if(is_null($state)) {
          $notes = $row->rules . " offenses exceeds limit of $max_offenses in " . format_interval($look_back);
          autoban_add_rule(array('expires' => $expires, 'mask' => $row->mask, 'notes' => $notes, 'reason' => AUTOBAN_REASON_REPEAT));
        }
      }
    } // end of if repeat offenders

    // Now get the number of rows we traversed last time. This is an optimisation
    // so that we don't fetch hundreds of rows from the DB when we don't need to.
    // The idea is that if we step through the "top hitters" and get 25 rows
    // in before we drop below our threshold, then next time, we'll only ask for
    // 50 rows.
    $last_max_rows = variable_get('autoban_last_max_rows', 100);
    $max_rows = $last_max_rows * 2;

    // Now get our thresholds
    $max_hits_threshold = variable_get('autoban_max_hits_threshold', 360);
    $time_threshold = variable_get('autoban_time_threshold', 3600);

    // Now work out how many hits an IP must have made since our last cron run
    // to be over the threshold...
    // If we're in incremental mode, then we work this out based on the last
    // Cron run time (otherwise we just use the last $time_threshold minutes of
    // accesslog). For Incremental, we have to multiply up the hits threshold
    // to match the time difference since the last cron run
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

    // Work out when they should be allowed back on (if we ban them)
    $ban_time = variable_get('autoban_ban_time', 3600);
    $expires = $time_now + $ban_time;
    // We decrease the expiry time by a tiny bit to stop any "resonance" with
    // cron runs. Eg. if cron runs hourly, and we ban someone at 12.00 for an hour,
    // then we want to remove them at 13.00 - we might miss them if cron runs for
    // a few seconds, so we just remove one minute from the ban time
    $expires = $expires - 60;
    // Just make sure we don't do something stupid looking. This can (almost)
    // not ever happen, but it's as well to make sure ;-)
    if($expires < $time_now) {
      $expires = $time_now;
    }
    
    // Now head into the DB, and pull out all IPs and how many hits they've made
    $result = db_query("SELECT COUNT(uid) AS hits, hostname FROM {accesslog} where timestamp>%d GROUP BY hostname ORDER BY hits DESC LIMIT %d", $point_in_time, $max_rows);

    $exec_time = $time_end - $time_start;
    $exec_time = sprintf("%.2f", $exec_time * 1000);

    // Remember how many rows we actually looked into...
    $rows_used = 0;
    while($row = db_fetch_object($result)) {
      $rows_used++;
      if($row->hits < $hits_required) {
        // This one's below the threshold, so stop processing any more rows
        break;
      }

      // This looks like a target for blocking
      $state = autoban_is_denied('host', $row->hostname);
      if(is_null($state)) {
        // Not white or black listed yet
        if(variable_get('autoban_tt_challenge',0)) {
          // We're issuing challenges, so add a challenge, not a hard ban rule
          if(!autoban_is_challenged($row->hostname)) {
            autoban_challenge($row->hostname, $row->hits . " hits exceeds limit of $max_hits_threshold in " . format_interval($time_threshold));
          }
        } else {
          // Do a hard-denial
          $notes = $row->hits . " hits exceeds limit of $max_hits_threshold in " . format_interval($time_threshold);
          autoban_add_rule(array('expires' => $expires, 'mask' => $row->hostname, 'status' => 0, 'notes' => $notes, 'reason' => AUTOBAN_REASON_OVER_LIMIT));
        }
      }
    }

    // If rows_used < 5 then pretend we used 5
    if($rows_used < 5) {
      $rows_used = 5;
    }

    // Now see if the Mollom module exists, if it does, then call it's library
    // Mollom is an anti-spam module available at http://drupal.org/project/mollom
    if(module_exists('mollom')) {
      include_once('lib/autoban.mollom.inc');
      autoban_mollom_cron();
    }

    // Now see if the beacon module exists, if it does, then call it's library
    // Beacon is a module produced by Pre-Emptive Limited (www.pre-emptive.net)
    // It is not required for autoban, but can be used with it if available
    if(module_exists('beacon')) {
      include_once('lib/autoban.beacon.inc');
      autoban_beacon_cron();
    }

    // Now remember everything for next time...
    variable_set('autoban_last_max_rows', $rows_used);
  } // end of if create_rules

  return;
}

// The following line is for Vim users - please don't delete it.
// vim: set filetype=php expandtab tabstop=2 shiftwidth=2:
