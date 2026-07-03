--TEST--
Statgrab is not serializable (PHP 8.1+)
--SKIPIF--
<?php
if (PHP_VERSION_ID < 80100) echo "skip requires PHP 8.1+ (ZEND_ACC_NOT_SERIALIZABLE)\n";
?>
--EXTENSIONS--
statgrab
--FILE--
<?php
try {
    serialize(new Statgrab());
    echo "FAIL: serialize allowed\n";
} catch (\Exception $e) {
    echo "serialize blocked: " . $e->getMessage() . "\n";
}

$blocked = false;
try {
    if (@unserialize('O:8:"Statgrab":0:{}') === false) {
        $blocked = true;
    }
} catch (\Throwable $e) {
    $blocked = true;
}
echo $blocked ? "unserialize blocked\n" : "FAIL: unserialize allowed\n";
?>
--EXPECTF--
serialize blocked: %s
unserialize blocked
