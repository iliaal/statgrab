--TEST--
sg_user_stats: per-user records (BC break vs 2006: was a flat username array)
--EXTENSIONS--
statgrab
--FILE--
<?php
$us = sg_user_stats();
if (!is_array($us)) {
    echo "FAIL: not array\n";
    exit;
}
if (count($us) === 0) {
    echo "no_users\n";
} else {
    $row = $us[0];
    foreach (['login_name', 'device', 'hostname', 'record_id'] as $k) {
        if (!is_string($row[$k] ?? null)) {
            echo "FAIL: $k not string\n";
        }
    }
    foreach (['pid', 'login_time', 'systime'] as $k) {
        if (!is_int($row[$k] ?? null)) {
            echo "FAIL: $k not int\n";
        }
    }
    echo "ok\n";
}
echo "DONE\n";
?>
--EXPECTF--
%s
DONE
