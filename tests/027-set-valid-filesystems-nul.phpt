--TEST--
2.1: sg_set_valid_filesystems rejects embedded NUL bytes with ValueError
--EXTENSIONS--
statgrab
--FILE--
<?php
try {
    sg_set_valid_filesystems(["ext4", "xf\0s"]);
    echo "FAIL: no exception\n";
} catch (\ValueError $e) {
    echo "throws ValueError\n";
}

/* OO mirror */
try {
    (new Statgrab())->setValidFilesystems(["a\0b"]);
    echo "FAIL: no method exception\n";
} catch (\ValueError $e) {
    echo "method throws ValueError\n";
}
?>
--EXPECT--
throws ValueError
method throws ValueError
