--TEST--
No-arg functions reject extra arguments
--EXTENSIONS--
statgrab
--FILE--
<?php
try {
    sg_load_stats('extra');
    echo "FAIL: no exception\n";
} catch (\ArgumentCountError $e) {
    echo "throws ArgumentCountError\n";
}

try {
    (new Statgrab())->load('extra');
    echo "FAIL: no method exception\n";
} catch (\ArgumentCountError $e) {
    echo "method throws ArgumentCountError\n";
}
?>
--EXPECT--
throws ArgumentCountError
method throws ArgumentCountError
