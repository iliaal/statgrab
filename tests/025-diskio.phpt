--TEST--
sg_diskio_stats + diff: row shape (read/written/time_frame)
--EXTENSIONS--
statgrab
--FILE--
<?php
foreach (['sg_diskio_stats', 'sg_diskio_stats_diff'] as $fn) {
    $d = $fn();
    if (!is_array($d)) {
        echo "FAIL: $fn not array\n";
        continue;
    }
    foreach ($d as $name => $row) {
        foreach (['read', 'written', 'time_frame'] as $k) {
            if (!is_int($row[$k] ?? null)) {
                echo "FAIL: $fn [$name] $k not int\n";
            }
        }
    }
}
echo "DONE\n";
?>
--EXPECT--
DONE
