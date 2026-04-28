--TEST--
sg_process_stats: sort order + limit
--EXTENSIONS--
statgrab
--FILE--
<?php
$by_pid = sg_process_stats(SG_PS_SORT_PID);
$pids = array_column($by_pid, 'pid');
$sorted = $pids;
sort($sorted, SORT_NUMERIC);
echo $pids === $sorted ? "sorted_by_pid\n" : "FAIL sort\n";

$limited = sg_process_stats(SG_PS_SORT_PID, 3);
echo count($limited) === 3 ? "limit_3\n" : "FAIL limit got " . count($limited) . "\n";

/* Invalid sort value emits warning + returns false */
$r = @sg_process_stats(99);
var_dump($r);

/* No sort -> no sort applied, no warning */
$nosort = sg_process_stats();
echo is_array($nosort) ? "nosort_ok\n" : "FAIL nosort\n";

echo "DONE\n";
?>
--EXPECT--
sorted_by_pid
limit_3
bool(false)
nosort_ok
DONE
