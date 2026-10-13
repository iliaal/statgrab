<?php
/* Swaps the process-global valid-filesystems list as fast as possible. */
$types = explode(',', getenv('SG_ZTS_FS_TYPES'));
$sets = [
    array_merge($types, ['sg-zts-none-a', 'sg-zts-none-b']),
    [$types[0]],
    array_reverse($types),
];
$end = microtime(true) + $GLOBALS['SG_SECONDS'];
$n = 0;
while (microtime(true) < $end) {
    if (sg_set_valid_filesystems($sets[$n++ % 3]) !== true) {
        echo "FAIL setter: sg_set_valid_filesystems returned non-true\n";
        return;
    }
}
echo "ok setter $n\n";
