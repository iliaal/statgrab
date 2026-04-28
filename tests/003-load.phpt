--TEST--
sg_load_stats: 1/5/15 minute averages
--EXTENSIONS--
statgrab
--FILE--
<?php
$l = sg_load_stats();
var_dump(is_array($l));
foreach (['min1', 'min5', 'min15'] as $k) {
    if (!is_float($l[$k] ?? null)) {
        echo "FAIL: $k not float\n";
    }
    if (($l[$k] ?? -1) < 0.0) {
        echo "FAIL: $k negative\n";
    }
}
echo "DONE\n";
?>
--EXPECT--
bool(true)
DONE
