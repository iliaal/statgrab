--TEST--
sg_process_stats: cpu_percent stays finite and non-negative for new processes
--EXTENSIONS--
statgrab
--SKIPIF--
<?php
if (!function_exists('proc_open')) die('skip proc_open unavailable');
?>
--FILE--
<?php
$bad = 0;
for ($i = 0; $i < 200; $i++) {
    $child = proc_open([PHP_BINARY, '-n', '-r', 'usleep(20000);'], [], $pipes);
    foreach ([null, SG_PS_SORT_CPU] as $mode) {
        foreach (sg_process_stats($mode) as $row) {
            $c = $row['cpu_percent'];
            if (!is_float($c) || !is_finite($c) || $c < 0.0) {
                $bad++;
            }
        }
    }
    proc_close($child);
}
var_dump($bad);
?>
--EXPECT--
int(0)
