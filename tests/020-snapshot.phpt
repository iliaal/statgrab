--TEST--
2.1: sg_snapshot returns true; OO Statgrab::snapshot mirrors it
--EXTENSIONS--
statgrab
--FILE--
<?php
var_dump(sg_snapshot());
var_dump((new Statgrab())->snapshot());

/* A snapshot seeds the diff sources: LAST_DIFF must serve an array now. */
$last = sg_cpu_percent_usage(Statgrab::CPU_PERCENT_LAST_DIFF);
echo is_array($last) ? "last_diff_array\n" : "FAIL last_diff\n";

/* systime advances across snapshots. */
$a = sg_load_stats();
sg_snapshot();
$b = sg_load_stats();
echo (is_array($a) && is_array($b) && $b['systime'] >= $a['systime'])
    ? "systime_advances\n" : "FAIL systime\n";

/* time_frame advances across snapshots. */
$p1 = sg_page_stats();
sg_snapshot();
$p2 = sg_page_stats();
echo (is_array($p1) && is_array($p2) && $p2['time_frame'] >= $p1['time_frame'])
    ? "time_frame_advances\n" : "FAIL time_frame\n";
?>
--EXPECT--
bool(true)
bool(true)
last_diff_array
systime_advances
time_frame_advances
