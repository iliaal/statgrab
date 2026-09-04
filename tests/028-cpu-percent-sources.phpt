--TEST--
sg_cpu_percent_usage: LAST_DIFF / NEW_DIFF sources return arrays with shape
--EXTENSIONS--
statgrab
--FILE--
<?php
function fail(string $msg): void { fwrite(STDERR, "FAIL: $msg\n"); exit(1); }
sg_snapshot();
$want = ['user', 'kernel', 'idle', 'iowait', 'swap', 'nice', 'previous_run'];
foreach ([Statgrab::CPU_PERCENT_LAST_DIFF, Statgrab::CPU_PERCENT_NEW_DIFF] as $src) {
    $r = sg_cpu_percent_usage($src);
    if (!is_array($r)) {
        fail("src $src not array");
    }
    if (count($r) !== 7) {
        fail("src $src key count " . count($r));
    }
    foreach ($want as $k) {
        if (!array_key_exists($k, $r)) {
            fail("src $src missing $k");
        }
    }
    if (array_keys($r) !== $want) {
        fail("src $src key shape");
    }
    foreach (['user', 'kernel', 'idle', 'iowait', 'swap', 'nice'] as $k) {
        if (!is_float($r[$k])) {
            fail("src $src $k not float");
        }
    }
    if (!is_int($r['previous_run'])) {
        fail("src $src previous_run not int");
    }
    echo "src $src: array\n";
}
echo "DONE\n";
?>
--EXPECTF--
src 1: array
src 2: array
DONE
