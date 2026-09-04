--TEST--
sg_network_iface_stats: link state per interface
--EXTENSIONS--
statgrab
--FILE--
<?php
function fail(string $msg): void { fwrite(STDERR, "FAIL: $msg\n"); exit(1); }
$ifs = sg_network_iface_stats();
if (!is_array($ifs)) {
    fail("sg_network_iface_stats not array");
}
if (count($ifs) === 0) {
    echo "no_ifaces\n";
} else {
    $valid_duplex = [Statgrab::DUPLEX_FULL, Statgrab::DUPLEX_HALF, Statgrab::DUPLEX_UNKNOWN];
    foreach ($ifs as $name => $row) {
        if (!is_string($name) || $name === '') {
            fail("bad key '$name'");
        }
        foreach (['speed', 'factor', 'duplex', 'systime'] as $k) {
            if (!is_int($row[$k] ?? null)) {
                fail("$name.$k not int");
            }
        }
        if (!is_bool($row['active'] ?? null)) {
            fail("$name.active not bool");
        }
        if (!in_array($row['duplex'], $valid_duplex, true)) {
            fail("$name.duplex unknown value");
        }
    }
    echo "ok\n";
}
echo "DONE\n";
?>
--EXPECTF--
%s
DONE
