<?php
$sg = new Statgrab();
$calls = [
    ['sg_diskio_stats', 'read'],
    ['sg_diskio_stats_diff', 'read'],
    [fn() => $sg->disks(), 'read'],
    [fn() => $sg->disks(true), 'read'],
    ['sg_network_stats', 'sent'],
    ['sg_network_stats_diff', 'sent'],
    [fn() => $sg->network(), 'sent'],
    [fn() => $sg->network(true), 'sent'],
    ['sg_network_iface_stats', 'speed'],
    [fn() => $sg->interfaces(), 'speed'],
];
// Collisions include normalized names, numeric fallback keys, missing/empty
// names, and an occupied fallback candidate after append-index exhaustion.
$keys = [0, 1, 2, 3, '02', 4, -1, 5, 11, PHP_INT_MAX, 'sda', 12, 13, '', 14];
foreach ($calls as $i => [$call, $field]) {
    $rows = $call();
    if (!is_array($rows) || array_column($rows, $field) !== range(0, 14)
        || array_keys($rows) !== $keys) {
        fwrite(STDERR, "FAIL: named rows, call $i\n" . var_export($rows, true) . "\n");
        exit(1);
    }
}
echo "PASS: all ten disk/network/interface entry points preserve named rows\n";
