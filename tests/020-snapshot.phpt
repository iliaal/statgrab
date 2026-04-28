--TEST--
2.1: sg_snapshot returns true; OO Statgrab::snapshot mirrors it
--EXTENSIONS--
statgrab
--FILE--
<?php
var_dump(sg_snapshot());
var_dump((new Statgrab())->snapshot());
?>
--EXPECT--
bool(true)
bool(true)
