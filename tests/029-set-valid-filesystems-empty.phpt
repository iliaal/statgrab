--TEST--
2.1: sg_set_valid_filesystems rejects empty array with ValueError
--EXTENSIONS--
statgrab
--FILE--
<?php
try {
    sg_set_valid_filesystems([]);
    echo "FAIL: no exception\n";
} catch (\ValueError $e) {
    echo "throws ValueError\n";
}

/* OO mirror */
try {
    (new Statgrab())->setValidFilesystems([]);
    echo "FAIL: no method exception\n";
} catch (\ValueError $e) {
    echo "method throws ValueError\n";
}

/* Rejection must be side-effect free: the list is still intact. */
$fs = sg_valid_filesystems();
$shape_ok = is_array($fs) && count($fs) > 0;
foreach ((array)$fs as $entry) {
    if (!is_string($entry) || $entry === '') { $shape_ok = false; break; }
}
echo $shape_ok ? "shape_intact\n" : "FAIL intact\n";
if (PHP_OS_FAMILY === 'Linux') {
    echo (is_array($fs) && count($fs) >= 5) ? "list_intact\n" : "FAIL intact count\n";
} else {
    echo "list_intact NA\n";
}
echo "DONE\n";
?>
--EXPECTF--
throws ValueError
method throws ValueError
shape_intact
%s
DONE
