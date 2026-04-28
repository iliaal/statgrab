--TEST--
sg_network_stats: per-interface I/O counters keyed by ifname
--EXTENSIONS--
statgrab
--FILE--
<?php
$n = sg_network_stats();
if (!is_array($n)) {
    echo "FAIL: not array\n";
    exit;
}
if (count($n) === 0) {
    echo "no_ifaces\n";
} else {
    foreach ($n as $name => $row) {
        if (!is_string($name) || $name === '') {
            echo "FAIL: bad key '$name'\n";
        }
        foreach (['sent', 'received', 'packets_received', 'packets_transmitted',
                  'receive_errors', 'transmit_errors', 'collisions', 'time_frame'] as $k) {
            if (!is_int($row[$k] ?? null)) {
                echo "FAIL: $name.$k not int\n";
            }
        }
    }
    $d = sg_network_stats_diff();
    echo is_array($d) ? "diff_array\n" : "FAIL diff\n";
}
echo "DONE\n";
?>
--EXPECTF--
%s
DONE
