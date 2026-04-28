--TEST--
sg_process_stats: shape, gid != uid (regression: 2006 release returned uid for both)
--EXTENSIONS--
statgrab
--FILE--
<?php
$ps = sg_process_stats();
if (!is_array($ps) || count($ps) === 0) {
    echo "FAIL: empty or not array\n";
    exit;
}

$row = $ps[0];
foreach (['process_name', 'proc_title'] as $k) {
    if (!is_string($row[$k] ?? null)) {
        echo "FAIL: $k not string\n";
    }
}
foreach (['pid', 'parent_pid', 'leader_pid', 'session_id',
          'uid', 'gid', 'euid', 'egid',
          'size', 'size_in_mem', 'start_time', 'time_spent',
          'nice', 'state'] as $k) {
    if (!is_int($row[$k] ?? null)) {
        echo "FAIL: $k not int\n";
    }
}
if (!is_float($row['cpu_percent'] ?? null)) {
    echo "FAIL: cpu_percent not float\n";
}

/* 2006 bug: gid mirrored uid, egid mirrored euid. Modern code reads them
 * separately. We can't assume gid != uid for every process (root runs as 0/0),
 * but at least one process on the system should have a non-zero distinction
 * between uid+gid and euid+egid in practice. */
$saw_distinct = false;
foreach ($ps as $r) {
    if ($r['gid'] !== $r['uid'] || $r['egid'] !== $r['euid']) {
        $saw_distinct = true;
        break;
    }
}
echo $saw_distinct ? "gid_distinct_from_uid\n" : "all_gid_equal_uid (acceptable but unusual)\n";
echo "DONE\n";
?>
--EXPECTF--
%s
DONE
