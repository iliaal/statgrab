--TEST--
sg_fs_stats: filesystems list with shape
--EXTENSIONS--
statgrab
--FILE--
<?php
$fs = sg_fs_stats();
if (!is_array($fs)) {
    echo "FAIL: not array\n";
    exit;
}
if (count($fs) === 0) {
    echo "SKIP no fs\n";
    exit;
}
$row = $fs[0];
foreach (['device_name', 'device_canonical', 'fs_type', 'mnt_point'] as $k) {
    if (!is_string($row[$k] ?? null)) {
        echo "FAIL: $k not string\n";
    }
}
foreach (['device_type', 'size', 'used', 'free', 'avail',
          'total_inodes', 'used_inodes', 'free_inodes', 'avail_inodes',
          'io_size', 'block_size', 'total_blocks', 'free_blocks',
          'used_blocks', 'avail_blocks', 'systime'] as $k) {
    if (!is_int($row[$k] ?? null)) {
        echo "FAIL: $k not int\n";
    }
}
echo "DONE\n";
?>
--EXPECT--
DONE
