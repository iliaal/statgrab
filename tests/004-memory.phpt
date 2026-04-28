--TEST--
sg_memory_stats: memory totals
--EXTENSIONS--
statgrab
--FILE--
<?php
$m = sg_memory_stats();
var_dump(is_array($m));
foreach (['total', 'free', 'used', 'cache'] as $k) {
    if (!is_int($m[$k] ?? null)) {
        echo "FAIL: $k not int\n";
    }
    if ($m[$k] < 0) {
        echo "FAIL: $k negative\n";
    }
}
echo $m['total'] > 0                   ? "total > 0\n"           : "FAIL total\n";
echo ($m['used'] + $m['free']) <= $m['total'] * 2
                                       ? "used+free sane\n"      : "FAIL used+free\n";
echo "DONE\n";
?>
--EXPECT--
bool(true)
total > 0
used+free sane
DONE
