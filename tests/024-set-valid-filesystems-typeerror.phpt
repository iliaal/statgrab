--TEST--
2.1: sg_set_valid_filesystems throws TypeError on non-string array entry
--EXTENSIONS--
statgrab
--FILE--
<?php
try {
    sg_set_valid_filesystems(['ext4', 42, 'xfs']);
    echo "FAIL: no exception\n";
} catch (\TypeError $e) {
    echo "throws TypeError\n";
}

/* OO mirror */
try {
    (new Statgrab())->setValidFilesystems(['ext4', null]);
    echo "FAIL: no method exception\n";
} catch (\TypeError $e) {
    echo "method throws TypeError\n";
}
?>
--EXPECT--
throws TypeError
method throws TypeError
