--TEST--
2.1: systime field is present on mem/swap/load/process_stats (Tier A)
--EXTENSIONS--
statgrab
--FILE--
<?php
$now = time();

$mem = sg_memory_stats();
echo is_int($mem['systime']) ? "mem.systime int\n" : "FAIL mem\n";

$swap = sg_swap_stats();
echo is_int($swap['systime']) ? "swap.systime int\n" : "FAIL swap\n";

$load = sg_load_stats();
echo is_int($load['systime']) ? "load.systime int\n" : "FAIL load\n";

/* All four samples were taken within the same second of wall clock. */
foreach ([$mem, $swap, $load] as $name => $sample) {
    if (abs($sample['systime'] - $now) > 5) {
        echo "FAIL: systime drift > 5s\n";
    }
}

$ps = sg_process_stats();
foreach (['context_switches', 'voluntary_context_switches',
          'involuntary_context_switches', 'systime'] as $f) {
    if (!is_int($ps[0][$f] ?? null)) {
        echo "FAIL: process.$f not int\n";
    }
}
echo "process new fields ok\n";
echo "DONE\n";
?>
--EXPECT--
mem.systime int
swap.systime int
load.systime int
process new fields ok
DONE
