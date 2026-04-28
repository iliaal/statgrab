--TEST--
sg_process_count: aggregate process counters
--EXTENSIONS--
statgrab
--FILE--
<?php
$p = sg_process_count();
var_dump(is_array($p));
foreach (['total', 'running', 'sleeping', 'stopped', 'zombie', 'unknown', 'systime'] as $k) {
    if (!is_int($p[$k] ?? null)) {
        echo "FAIL: $k not int\n";
    }
    if ($p[$k] < 0) {
        echo "FAIL: $k negative\n";
    }
}
echo $p['total'] >= 1 ? "at least one process\n" : "FAIL total < 1\n";
echo "DONE\n";
?>
--EXPECT--
bool(true)
at least one process
DONE
