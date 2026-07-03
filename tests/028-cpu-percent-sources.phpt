--TEST--
sg_cpu_percent_usage: LAST_DIFF / NEW_DIFF sources return array|false with shape
--EXTENSIONS--
statgrab
--FILE--
<?php
sg_snapshot();
foreach ([Statgrab::CPU_PERCENT_LAST_DIFF, Statgrab::CPU_PERCENT_NEW_DIFF] as $src) {
    $r = @sg_cpu_percent_usage($src);
    if ($r === false) {
        echo "src $src: false\n";
        continue;
    }
    if (!is_array($r)) {
        echo "FAIL: src $src not array|false\n";
        continue;
    }
    foreach (['user', 'kernel', 'idle', 'iowait', 'swap', 'nice', 'previous_run'] as $k) {
        if (!array_key_exists($k, $r)) {
            echo "FAIL: src $src missing $k\n";
        }
    }
    echo "src $src: array\n";
}
echo "DONE\n";
?>
--EXPECTF--
src 1: %s
src 2: %s
DONE
