--TEST--
sg_cpu_percent_usage: CPU percent breakdown
--EXTENSIONS--
statgrab
--FILE--
<?php
$c = sg_cpu_percent_usage();
var_dump(is_array($c));
foreach (['user', 'kernel', 'idle', 'iowait', 'swap', 'nice'] as $k) {
    if (!is_float($c[$k] ?? null)) {
        echo "FAIL: $k not float\n";
    }
    if ($c[$k] < 0.0 || $c[$k] > 100.0) {
        echo "FAIL: $k out of [0, 100]\n";
    }
}
if (!is_int($c['previous_run'] ?? null)) echo "FAIL: previous_run not int\n";
echo "DONE\n";
?>
--EXPECT--
bool(true)
DONE
