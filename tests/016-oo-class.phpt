--TEST--
Statgrab class: OO surface mirrors procedural functions
--EXTENSIONS--
statgrab
--FILE--
<?php
$sg = new Statgrab();

/* Each accessor returns the same shape as its procedural sibling. */
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
    if (gettype($a) !== gettype($b)) {
        echo "FAIL: $method/$func type mismatch\n";
        continue;
    }
    if (is_array($a) && is_array($b)) {
        if (array_keys($a) !== array_keys($b)) {
            echo "FAIL: $method/$func key mismatch\n";
        }
    }
}

/* diff variants */
foreach (['disks', 'network', 'pages'] as $m) {
    $cum  = $sg->$m(false);
    $dif  = $sg->$m(true);
    if (gettype($cum) !== gettype($dif)) {
        echo "FAIL: $m diff variant type\n";
    }
}

/* processes() */
$top = $sg->processes(Statgrab::SORT_PID, 5);
echo (is_array($top) && count($top) === 5) ? "top_pids_5\n" : "FAIL processes\n";

echo "DONE\n";
?>
--EXPECT--
top_pids_5
DONE
