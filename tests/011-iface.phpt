--TEST--
sg_network_iface_stats: link state per interface
--EXTENSIONS--
statgrab
--FILE--
<?php
$ifs = sg_network_iface_stats();
if (!is_array($ifs)) {
    echo "FAIL: not array\n";
    exit;
}
if (count($ifs) === 0) {
    echo "no_ifaces\n";
} else {
    $valid_duplex = [Statgrab::DUPLEX_FULL, Statgrab::DUPLEX_HALF, Statgrab::DUPLEX_UNKNOWN];
    foreach ($ifs as $name => $row) {
        if (!is_string($name) || $name === '') {
            echo "FAIL: bad key '$name'\n";
        }
        foreach (['speed', 'factor', 'duplex', 'systime'] as $k) {
            if (!is_int($row[$k] ?? null)) {
                echo "FAIL: $name.$k not int\n";
            }
        }
        if (!is_bool($row['active'] ?? null)) {
            echo "FAIL: $name.active not bool\n";
        }
        if (!in_array($row['duplex'], $valid_duplex, true)) {
            echo "FAIL: $name.duplex unknown value\n";
        }
    }
    echo "ok\n";
}
echo "DONE\n";
?>
--EXPECTF--
%s
DONE
