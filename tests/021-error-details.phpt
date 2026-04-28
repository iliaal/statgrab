--TEST--
2.1: sg_error_details returns false when no error pending
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

echo "DONE\n";
?>
--EXPECT--
bool(false)
bool(false)
DONE
