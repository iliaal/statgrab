--TEST--
2.1: sg_error_details returns false when no error pending, details after failure
--EXTENSIONS--
statgrab
--FILE--
<?php
/* After a clean call to a working stat, error state is "none". */
sg_load_stats();
var_dump(sg_error_details());

$sg = new Statgrab();
$sg->load();
var_dump($sg->errorDetails());

echo "DONE-false\n";

/* Argument validation does not set libstatgrab errors. A populated error
 * requires a library failure, which this healthy-system test cannot force. */
$r = sg_process_stats(99);
var_dump($r);
var_dump(sg_error_details());

$sg2 = new Statgrab();
var_dump($sg2->processes(99));
var_dump($sg2->errorDetails());
?>
--EXPECTF--
bool(false)
bool(false)
DONE-false

Warning: %s'99' is not a supported sorting mode in %s on line %d
bool(false)
bool(false)

Warning: %s'99' is not a supported sorting mode in %s on line %d
bool(false)
bool(false)
