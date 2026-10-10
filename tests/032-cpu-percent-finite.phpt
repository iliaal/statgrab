--TEST--
CPU diff percentages stay finite and JSON-encodable when no ticks elapse
--EXTENSIONS--
statgrab
--FILE--
<?php
$sg = new Statgrab();
$fields = ['user', 'kernel', 'idle', 'iowait', 'swap', 'nice'];
foreach (['sg_cpu_percent_usage', [$sg, 'cpu']] as $call) {
    /* Rapid NEW_DIFF calls can contain zero ticks. LAST_DIFF reuses that
     * same sample. Do not sleep: that would hide the zero-denominator case. */
    for ($i = 0; $i < 200; $i++) {
        foreach ([Statgrab::CPU_PERCENT_NEW_DIFF, Statgrab::CPU_PERCENT_LAST_DIFF] as $source) {
            $sample = $call($source);
            if (!is_array($sample) || array_keys($sample) !== array_merge($fields, ['previous_run'])) {
                die("FAIL: sample shape\n");
            }
            foreach ($fields as $field) {
                if (!is_float($sample[$field]) || !is_finite($sample[$field])) {
                    die("FAIL: non-finite $field\n");
                }
            }
            if (!is_int($sample['previous_run'])) {
                die("FAIL: previous_run type\n");
            }
            if (json_encode($sample) === false) {
                die("FAIL: JSON encoding\n");
            }
        }
    }
    echo is_string($call) ? "procedural OK\n" : "OO OK\n";
}
?>
--EXPECT--
procedural OK
OO OK
