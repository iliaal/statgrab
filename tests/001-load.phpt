--TEST--
statgrab: extension loads, version, classes and constants exposed
--EXTENSIONS--
statgrab
--FILE--
<?php
var_dump(extension_loaded('statgrab'));
var_dump(phpversion('statgrab'));
var_dump(class_exists('Statgrab', false));
var_dump(defined('SG_FULL_DUPLEX'));
var_dump(SG_FULL_DUPLEX === Statgrab::DUPLEX_FULL);
var_dump(SG_PS_SORT_NAME === Statgrab::SORT_NAME);
var_dump(SG_PROCESS_STATE_RUNNING === Statgrab::STATE_RUNNING);
?>
--EXPECTF--
bool(true)
string(%d) "%d.%d.%s"
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
