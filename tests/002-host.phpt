--TEST--
sg_general_stats: host info shape
--EXTENSIONS--
statgrab
--FILE--
<?php
$h = sg_general_stats();
var_dump(is_array($h));
foreach (['os_name', 'os_release', 'os_version', 'platform', 'hostname'] as $k) {
    if (!is_string($h[$k] ?? null)) {
        echo "FAIL: $k not string\n";
    }
}
foreach (['bitwidth', 'host_state', 'ncpus', 'maxcpus', 'uptime', 'systime'] as $k) {
    if (!is_int($h[$k] ?? null)) {
        echo "FAIL: $k not int\n";
    }
}
echo $h['uptime'] >= 0 ? "uptime non-negative\n" : "FAIL: negative uptime\n";
echo $h['ncpus'] >= 1   ? "ncpus >= 1\n"          : "FAIL: ncpus < 1\n";
echo "DONE\n";
?>
--EXPECT--
bool(true)
uptime non-negative
ncpus >= 1
DONE
