# Autoban: There's a Speed Limit on the Autoban

Autoban is an extension to the Drupal core User module. It particularly provides extensions to the User Access Rules features.

Features include:

* Access rules can be manually created with an expiry date (or set to be permanent)
* Access rules can have comments added to aid in administration
* IP Deny rules can be created automatically if users exceed configurable thresholds
* Visitors exceeding thresholds can be 'challenged' to determine if they are human or not
* Persistent offenders can be handled separately by detecting repeat offences
* Co-ordinated subnet detection to stop 'IP hopping' by attackers
* Integration with the Mollom anti-spam module
* Integration with the Internet Information module 

## What Can You Do with Autoban?

With Autoban you can temporarily ban visitors that hit your site too hard. Often these sorts of visitors are 'bots' that provide little value to your site, yet consume resources by making numerous, rapid requests for long periods of time. With Autoban you can detect these visitors and deny them access to your site, saving your resources.

The Repeat Offenders feature means that it's possible to take action against persistent abusers. If an IP address repeatedly exceeds the thresholds for normal temporary banning, then it's possible to ban them for a longer period.

For example, normal thresholds may be configured to ban a user if they make more than 100 requests in a 60 minute period. If they exceed this, then they may be banned for 30 minutes. If after the 30 minute banning time, they return and re-offend 5 more times then the system can ban them for a whole day.

Similarly, if you identify that a particular IP address is spamming your comments system, then it's possible to ban them manually. If they've been particularly aggressive, you may choose to ban them (or their entire network) for a week or a month.

Finally, you probably don't want to block yourself, so it's possible to add in 'whitelist' rules that ensure your machines are always allowed access, even if you exceed the configured thresholds.
