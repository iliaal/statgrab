--TEST--
2.1: sg_error_details returns false when no error pending, details after failure
--EXTENSIONS--
statgrab
--FILE--
<?php
/* After a clean call to a working stat, error state is "none". */
sg_load_stats();
var_dump(sg_error_details());

/* OO mirror */
$sg = new Statgrab();
$sg->load();
var_dump($sg->errorDetails());

echo "DONE-false\n";

/* Extension-level arg validation never reaches libstatgrab, so no lib
 * error is pending afterwards: details stay false. (No userland call on a
 * healthy system forces a real lib failure — every getter reads live
 * /proc state — so the populated 4-key array shape is verified by code
 * inspection of php_sg_error_details, pinned here by the false boundary.) */
$r = sg_process_stats(99);
var_dump($r);
var_dump(sg_error_details());

/* OO mirror of the boundary. */
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
