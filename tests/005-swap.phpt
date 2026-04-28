--TEST--
sg_swap_stats: swap totals (may be all-zero)
--EXTENSIONS--
statgrab
--FILE--
<?php
$s = sg_swap_stats();
var_dump(is_array($s));
foreach (['total', 'free', 'used'] as $k) {
    if (!is_int($s[$k] ?? null)) {
        echo "FAIL: $k not int\n";
    }
    if ($s[$k] < 0) {
        echo "FAIL: $k negative\n";
    }
}
echo "DONE\n";
?>
--EXPECT--
bool(true)
DONE
