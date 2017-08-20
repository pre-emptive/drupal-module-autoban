<?php

// $Id: autoban.challenges.inc,v 1.5 2008/10/15 11:58:45 ralph Exp $

// (C)2008 Pre-Emptive Limited. http://www.pre-emptive.net
// GNU Public Licenced (v2)
// Written by Ralph Bolton, September 2008

// This library actually issues challenges to users. That is, it displays
// a challenge that the user must pass to be accepted into the system.

// _autoban_make_challenge_path()
// Internal function, used to make a URL path that includes the
// 'secret' challenge reciever path, the magic key for this user and
// (if possible) a query string that contains the URL of the page
// that the user really wanted to go to (which is subsequently used for
// a 302 redirect).
function _autoban_make_challenge_path($path, $magickey, $do_dest = 1) {
  global $base_path;
  $link = $base_path . '/' . $path . '/' . $magickey;
  if(substr($link,0,2) == '//') {
    $link = substr($link, 1);
  }
  if($do_dest) {
    // Add a destination if it's available
    if(isset($_REQUEST['q'])) {
      $link .= '?destination=' . urlencode($_REQUEST['q']);
    }
  }
  // Returns something like /drupal/_autoban/hwyw/fhuqwhiry29ej?destination=/node/123
  return $link;
}

// _autoban_get_challenge_destination
// Internal function that tries to determine the URL of the page the user
// actually wanted to see before we challenged them.
function _autoban_get_challenge_destination() {
  if(isset($_REQUEST['q'])) {
    global $base_path;
    $destination = $base_path . check_plain($_REQUEST['q']);
    if(substr($destination,0,2) == '//') {
      $destination = substr($destination, 1);
    }
  } else {
    $destination = '/';
  }
  // Returns something like /node/123
  return $destination;
}

// autoban_issue_challenge()
// This is the function that actually displays a challenge to the user. It
// does so 'directly', so does not use Drupal content management. This means
// that the HTML generated is pretty basic. This is intentional, because the
// viewer is already (probably) a 'bot, and so we want to keep bandwidth to
// a minimum. For humans, the page is a bit basic, but should be perfectly
// usable. They shouldn't see it too often if we're configured properly.
// Challenges are pre-decided in autoban_challenge(). This function really
// just displays that challenge. This is called by hook_boot(), so does not
// have all of Drupa available (most notibly, t() is not available).
// Paramters:
// row : A database row from the autoban_hitlist table
function autoban_issue_challenge($row = NULL) {
  if(is_null($row)) {
    // Barf! We haven't been handed a DB row of info
    print "Internal Autoban error. Contact site administrator.";
    exit;
  }

  // All challenges are actually 403 errors. This stops crawlers indexing
  // them, and gives robots the information they need to stop hassling us
  // But 403 is invisible to a browser user
  header('HTTP/1.1 403 Forbidden');

  // Get the 'secret' path for the challenge receiver
  $path = variable_get('autoban_tt_path', '_autoban');

  switch($row->challenge) {
    case AUTOBAN_PLAIN_LINK:
      // Plain links are just simple hyperlinks to the challenge receiver
      // This is super-simple, but actually stops a lot of bots
      $link = _autoban_make_challenge_path($path, $row->magickey);
      print '<html><body>';
      print 'Please click <a href="' . $link . '">this link</a> to continue.';
      print '</body></html>';
      break;
    case AUTOBAN_JS_LINK:
      // A JS link requires a bit of Javascript execution to actually make
      // the request to the challenge receiver. This is still simple, and could
      // be extended so that it has to do some processing to 'decode' the magic
      // key into the real key. That would mean a 'bot would have to do some fairly
      // clever work to defeat the challenge. For now, this looks adequate.
      global $base_url;
      $link = _autoban_make_challenge_path($path, $row->magickey);
      $link = $base_url . $link;
      print '<html><head><script type="text/javascript">function click() { open(\'' . $link . '\', \'_self\'); }</script>';
      print '</head><body>';
      print 'Please click <a href="#" onclick="click()">this link</a> to continue.';
      print '</body></html>';
      break;
    case AUTOBAN_PLAIN_IMAGE:
      // Image challenges use the challenge receiver to deliver the browser an
      // image. When it does this, it whitelists the client, so the user can happily
      // navigate around the site as soon as the image loads. For this reason, we
      // deliver a plain HTML page, with the embedded image and a Meta Refresh
      // tag to the page the user really wanted. If for some reason the image didn't
      // actually whitelist the user, then the refresh will actually cause the user
      // be banned instantly (because the refresh will use up all of the "remaining"
      // counter). Again, this test isn't hard to defeat, but means robots would need
      // to download images.
      $link = _autoban_make_challenge_path($path, $row->magickey);
      $destination = _autoban_get_challenge_destination();
      print '<html><head><meta http-equiv="refresh" content="0;url=' . $destination . '">';
      print '<img src="' . $link . '" width="1" height="1">Please wait... (or <a href="' . $destination . '">click here</a>)';
      print '</body></html>';
      break;
    case AUTOBAN_JS_IMAGE:
      // JS image challenges are much like plain image challenges, except
      // the HTML for the image load is dynamically added by Javascript on page
      // load. This isn't hard to reverse engineer, so could be upgraded to
      // somehow scramble the HTML or link that are dynamically added.
      global $base_url;
      $link = _autoban_make_challenge_path($path, $row->magickey);
      $destination = _autoban_get_challenge_destination();
      print '<html><head><script type="text/javascript">function click() { open(\'' . $base_url . $destination . '\', \'_self\'); }</script>';
      print '</head><body onload="click()">';
      print 'Please wait... ';
      print '<script type="text/javascript">document.write(\'<img src="' . $link . '">\');</script>';
      print '</body></html>';
      break;
    case AUTOBAN_SIMPLE_MATHS:
      // A simple maths challenge asks the user to solve a maths question.
      // Eg. "What is 2 + 3?". For this, they have to POST the response back
      // to the challenge receiver. A robot could be constructed to do this
      // relatively easily, but these simple challenges stop the "generic"
      // bots that tend to be most prevelent.
      $link = _autoban_make_challenge_path($path, $row->magickey, 0);
      if(isset($_REQUEST['q'])) {
        $destination = check_plain($_REQUEST['q']);
      } else {
        $destination = '/';
      }
      print '<html><body>';
      print 'To continue, please answer this simple question:<br>What is ' . $row->question . '?';
      print '<form action="' . $link . '" method="post"><input type="hidden" name="destination" value="' . $destination . '"><input type="text" name="answer"><input type="submit" value="Answer"></form>';
      print '</body></html>';
      break;
    case AUTOBAN_RECAPTCHA:
      // ReCaptcha challenges are the most sophisticated challenge we have.
      // We use the ReCaptcha web service to display a textual challenge to
      // the user. These challenges are aguably the most annoying, but most
      // likely to defeat robots. Since we don't often show recaptchas, it's
      // unlikely they could be used in a "free porn for captcha" scam.
      $link = _autoban_make_challenge_path($path, $row->magickey, 0);
      if(isset($_REQUEST['q'])) {
        $destination = check_plain($_REQUEST['q']);
      } else {
        $destination = '/';
      }
      require_once(AUTOBAN_RECAPTCHA_LIBRARY . '/recaptchalib.php');
      $publickey = variable_get('autoban_recaptcha_publickey', '');
      print '<html><body>';
      print 'Please enter both words below in the box:';
      print '<form action="' . $link . '" method="post"><input type="hidden" name="destination" value="' . $destination . '">';
      print recaptcha_get_html($publickey);
      print 'Then click the button: <input type="submit" value="Submit"></form>';
      print '</body></html>';
      break;
  }
}

// The following line is for Vim users - please don't delete it.
// vim: set filetype=php expandtab tabstop=2 shiftwidth=2:
