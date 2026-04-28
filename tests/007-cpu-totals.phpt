--TEST--
sg_cpu_totals + sg_cpu_diff: jiffy counters
--EXTENSIONS--
statgrab
--FILE--
<?php
$c = sg_cpu_totals();
var_dump(is_array($c));
foreach (['user', 'kernel', 'idle', 'iowait', 'swap', 'nice', 'total',
          'context_switches', 'voluntary_context_switches',
          'involuntary_context_switches', 'syscalls', 'interrupts',
          'soft_interrupts', 'previous_run'] as $k) {
    if (!is_int($c[$k] ?? null)) {
        echo "FAIL: $k not int ($k => ", var_export($c[$k] ?? null, true), ")\n";
    }
}
$d = sg_cpu_diff();
var_dump(is_array($d));
echo "DONE\n";
?>
--EXPECT--
bool(true)
bool(true)
DONE
