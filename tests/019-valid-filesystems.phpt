--TEST--
2.1: sg_valid_filesystems / sg_set_valid_filesystems round-trip
--EXTENSIONS--
statgrab
--FILE--
<?php
$default = sg_valid_filesystems();
echo is_array($default) ? "got_default\n" : "FAIL default\n";

/* Shape holds everywhere: non-empty list of non-empty strings. */
$shape_ok = is_array($default) && count($default) > 0;
foreach ((array)$default as $fs) {
    if (!is_string($fs) || $fs === '') { $shape_ok = false; break; }
}
echo $shape_ok ? "shape_ok\n" : "FAIL shape\n";

/* Linux-only asserts: dense defaults including ext4. */
if (PHP_OS_FAMILY === 'Linux') {
    echo count($default) >= 5 ? "default_has_>=5\n" : "FAIL count\n";
    echo in_array('ext4', $default, true) ? "has_ext4\n" : "FAIL ext4\n";
} else {
    echo "default_has_>=5 NA\n";
    echo "has_ext4 NA\n";
}

/* Override to a tiny set, confirm round-trip. */
$ok = sg_set_valid_filesystems(['ext4', 'xfs']);
var_dump($ok);

$narrow = sg_valid_filesystems();
echo $narrow === ['ext4', 'xfs'] ? "narrow_set\n" : "FAIL narrow ".json_encode($narrow)."\n";

/* Restore the defaults so subsequent tests don't see a narrowed list. */
sg_set_valid_filesystems($default);

echo "DONE\n";
?>
--EXPECTF--
got_default
shape_ok
%s
%s
bool(true)
narrow_set
DONE
