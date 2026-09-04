--TEST--
sg_user_stats: per-user records (BC break vs 2006: was a flat username array)
--EXTENSIONS--
statgrab
--FILE--
<?php
function fail(string $msg): void { fwrite(STDERR, "FAIL: $msg\n"); exit(1); }
$us = sg_user_stats();
if (!is_array($us)) {
    fail("sg_user_stats not array");
}
if (count($us) === 0) {
    echo "no_users\n";
} else {
    $row = $us[0];
    foreach (['login_name', 'device', 'hostname', 'record_id'] as $k) {
        if (!is_string($row[$k] ?? null)) {
            fail("$k not string");
        }
    }
    foreach (['pid', 'login_time', 'systime'] as $k) {
        if (!is_int($row[$k] ?? null)) {
            fail("$k not int");
        }
    }
    echo "ok\n";
}
echo "DONE\n";
?>
--EXPECTF--
%s
DONE
