--TEST--
sg_page_stats and sg_page_stats_diff: pagein/pageout counters (regression: 2006 release swapped these)
--EXTENSIONS--
statgrab
--FILE--
<?php
$p = sg_page_stats();
$d = sg_page_stats_diff();

foreach (['p (cumulative)' => $p, 'd (diff)' => $d] as $label => $row) {
    if (!is_array($row)) {
        echo "FAIL $label not array\n";
        continue;
    }
    foreach (['pages_in', 'pages_out', 'time_frame'] as $k) {
        if (!is_int($row[$k] ?? null)) {
            echo "FAIL: $label.$k not int\n";
        }
        if ($row[$k] < 0) {
            echo "FAIL: $label.$k negative\n";
        }
    }
}

/* Cumulative >= diff for pagein on a steady-state system. The 2006 release
 * swapped the two; this asserts the fix. */
echo $p['pages_in'] >= $d['pages_in'] ? "cumulative_ge_diff\n" : "FAIL cumulative<diff\n";
echo "DONE\n";
?>
--EXPECT--
cumulative_ge_diff
DONE
