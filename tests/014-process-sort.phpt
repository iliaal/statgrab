--TEST--
sg_process_stats: sort order + limit
--EXTENSIONS--
statgrab
--FILE--
<?php
/* All 8 sort modes return arrays sorted by their key (direction-agnostic:
 * ascending or descending both prove the comparator ran). */
$modes = [
    SG_PS_SORT_NAME => 'process_name',
    SG_PS_SORT_PID  => 'pid',
    SG_PS_SORT_UID  => 'uid',
    SG_PS_SORT_GID  => 'gid',
    SG_PS_SORT_SIZE => 'size',
    SG_PS_SORT_RES  => 'size_in_mem',
    SG_PS_SORT_CPU  => 'cpu_percent',
    SG_PS_SORT_TIME => 'time_spent',
];
foreach ($modes as $mode => $key) {
    $rows = sg_process_stats($mode);
    if (!is_array($rows) || count($rows) === 0) {
        echo "FAIL sort $mode not array\n";
        continue;
    }
    $vals = array_column($rows, $key);
    $asc = $desc = true;
    for ($i = 1, $n = count($vals); $i < $n; $i++) {
        $c = ($key === 'process_name')
            ? strcmp((string)$vals[$i - 1], (string)$vals[$i])
            : ($vals[$i - 1] <=> $vals[$i]);
        if ($c > 0) $asc = false;
        if ($c < 0) $desc = false;
    }
    echo ($asc || $desc) ? "sorted_$mode\n" : "FAIL sort $mode not sorted\n";
}

/* limit 0 behaves like no limit. The table is live so two back-to-back
 * reads may straddle a fork/exit; allow a small churn tolerance. */
$full = sg_process_stats();
$zero = sg_process_stats(SG_PS_SORT_PID, 0);
$fc = is_array($full) ? count($full) : -1;
$zc = is_array($zero) ? count($zero) : -1;
echo ($fc >= 0 && abs($zc - $fc) <= 2) ? "limit_0_full\n" : "FAIL limit_0 got $zc want $fc\n";

/* Explicit null sort: warning-free unsuppressed call returning an array. */
$nosort = sg_process_stats(null);
echo is_array($nosort) ? "null_ok\n" : "FAIL null\n";

/* Invalid sort value emits warning + returns false */
$r = @sg_process_stats(99);
var_dump($r);

echo "DONE\n";
?>
--EXPECT--
sorted_0
sorted_1
sorted_2
sorted_3
sorted_4
sorted_5
sorted_6
sorted_7
limit_0_full
null_ok
bool(false)
DONE
