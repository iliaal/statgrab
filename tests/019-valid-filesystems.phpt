--TEST--
2.1: sg_valid_filesystems / sg_set_valid_filesystems round-trip
--EXTENSIONS--
statgrab
--FILE--
<?php
/* The defaults always include at least ext4 and tmpfs on Linux. */
$default = sg_valid_filesystems();
echo is_array($default) ? "got_default\n" : "FAIL default\n";
echo count($default) >= 5 ? "default_has_>=5\n" : "FAIL count\n";
echo in_array('ext4', $default, true) ? "has_ext4\n" : "FAIL ext4\n";

/* Override to a tiny set, confirm round-trip. */
$ok = sg_set_valid_filesystems(['ext4', 'xfs']);
var_dump($ok);

$narrow = sg_valid_filesystems();
echo $narrow === ['ext4', 'xfs'] ? "narrow_set\n" : "FAIL narrow ".json_encode($narrow)."\n";

/* Restore the defaults so subsequent tests don't see a narrowed list. */
sg_set_valid_filesystems($default);

echo "DONE\n";
?>
--EXPECT--
got_default
default_has_>=5
has_ext4
bool(true)
narrow_set
DONE
