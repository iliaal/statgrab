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

/* 2006 bug: gid mirrored uid, egid mirrored euid. Cross-check our own
 * process against the kernel's view via /proc/self/status (Linux, no
 * posix ext needed): statgrab must report the real uid/gid/euid/egid,
 * not uid twice. Off Linux (no /proc) the shape checks above still stand. */
if (is_readable('/proc/self/status')) {
    $status = file_get_contents('/proc/self/status');
    if (preg_match('/^Uid:\s+(\d+)\s+(\d+)/m', $status, $u) &&
        preg_match('/^Gid:\s+(\d+)\s+(\d+)/m', $status, $g)) {
        $want = ['uid' => (int)$u[1], 'euid' => (int)$u[2],
                 'gid' => (int)$g[1], 'egid' => (int)$g[2]];
        $mypid = getmypid();
        foreach ($ps as $r) {
            if ($r['pid'] === $mypid) {
                foreach ($want as $k => $v) {
                    if ($r[$k] !== $v) {
                        echo "FAIL: $k {$r[$k]} != /proc $v\n";
                    }
                }
                break;
            }
        }
    }
}
echo "DONE\n";
?>
--EXPECT--
DONE
