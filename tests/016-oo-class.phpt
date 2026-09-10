--TEST--
Statgrab class: OO surface mirrors procedural functions
--EXTENSIONS--
statgrab
--FILE--
<?php
$sg = new Statgrab();

$pairs = [
    'cpu'          => 'sg_cpu_percent_usage',
    'cpuStats'     => 'sg_cpu_totals',
    'cpuDiff'      => 'sg_cpu_diff',
    'filesystems'  => 'sg_fs_stats',
    'host'         => 'sg_general_stats',
    'load'         => 'sg_load_stats',
    'memory'       => 'sg_memory_stats',
    'swap'         => 'sg_swap_stats',
    'processCount' => 'sg_process_count',
    'users'        => 'sg_user_stats',
    'interfaces'   => 'sg_network_iface_stats',
];
foreach ($pairs as $method => $func) {
    $a = $sg->$method();
    $b = $func();
    if (!is_array($a) || !is_array($b)) {
        echo "FAIL: $method/$func not array\n";
        continue;
    }
    if (array_keys($a) !== array_keys($b)) {
        echo "FAIL: $method/$func key mismatch\n";
    }
}
echo "pairs_ok\n";

$diffs = [
    'disks'   => 'sg_diskio_stats_diff',
    'network' => 'sg_network_stats_diff',
    'pages'   => 'sg_page_stats_diff',
];
foreach ($diffs as $m => $func) {
    $oo = $sg->$m(true);
    $pc = $func();
    if (!is_array($oo) || !is_array($pc)) {
        echo "FAIL: $m diff not array\n";
        continue;
    }
    if (array_keys($oo) !== array_keys($pc)) {
        echo "FAIL: $m diff key mismatch\n";
        continue;
    }
    echo "{$m}_diff_ok\n";
}

$orig = sg_valid_filesystems();
if (!is_array($orig) || count($orig) === 0) {
    echo "FAIL: validFilesystems default not array\n";
} else {
    $narrow = array_slice($orig, 0, 2);
    if ($sg->setValidFilesystems($narrow) !== true) {
        echo "FAIL: OO setValidFilesystems\n";
    } elseif ($sg->validFilesystems() !== $narrow) {
        echo "FAIL: OO validFilesystems round-trip\n";
    } else {
        echo "oo_fs_roundtrip\n";
    }
    sg_set_valid_filesystems($orig);
    echo sg_valid_filesystems() === $orig ? "fs_restored\n" : "FAIL fs restore\n";
}

$top = $sg->processes(Statgrab::SORT_PID, 5);
echo (is_array($top) && count($top) === 5) ? "top_pids_5\n" : "FAIL processes\n";

echo "DONE\n";
?>
--EXPECT--
pairs_ok
disks_diff_ok
network_diff_ok
pages_diff_ok
oo_fs_roundtrip
fs_restored
top_pids_5
DONE
