<?php
/* sg_valid_filesystems() copies the list the setter frees. */
$end = microtime(true) + $GLOBALS['SG_SECONDS'];
$n = 0;
$sg = new Statgrab();
while (microtime(true) < $end) {
    $a = sg_valid_filesystems();
    $b = $sg->validFilesystems();
    if (!is_array($a) || !is_array($b)) {
        echo "FAIL list: valid filesystems returned false\n";
        return;
    }
    foreach ([$a, $b] as $list) {
        foreach ($list as $type) {
            if (!is_string($type) || $type === '') {
                echo "FAIL list: bad entry\n";
                return;
            }
        }
    }
    $n++;
}
echo "ok list $n\n";
