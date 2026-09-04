--TEST--
sg_network_stats: per-interface I/O counters keyed by ifname
--EXTENSIONS--
statgrab
--FILE--
<?php
function fail(string $msg): void { fwrite(STDERR, "FAIL: $msg\n"); exit(1); }
$n = sg_network_stats();
if (!is_array($n)) {
    fail("sg_network_stats not array");
}
if (count($n) === 0) {
    echo "no_ifaces\n";
} else {
    foreach ($n as $name => $row) {
        if (!is_string($name) || $name === '') {
            fail("bad key '$name'");
        }
        foreach (['sent', 'received', 'packets_received', 'packets_transmitted',
                  'receive_errors', 'transmit_errors', 'collisions', 'time_frame'] as $k) {
            if (!is_int($row[$k] ?? null)) {
                fail("$name.$k not int");
            }
        }
    }
    echo "ok\n";
}
$d = sg_network_stats_diff();
if (!is_array($d)) {
    fail("sg_network_stats_diff not array");
}
echo "DONE\n";
?>
--EXPECTF--
%s
DONE
