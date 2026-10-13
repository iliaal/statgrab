<?php
/* sg_fs_stats() filters mounts through the valid-filesystems list. */
$end = microtime(true) + $GLOBALS['SG_SECONDS'];
$n = 0;
$sg = new Statgrab();
while (microtime(true) < $end) {
    if (!is_array(sg_fs_stats()) || !is_array($sg->filesystems())) {
        echo "FAIL fs: filesystem stats returned false\n";
        return;
    }
    $n++;
}
echo "ok fs $n\n";
