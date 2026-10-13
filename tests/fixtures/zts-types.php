<?php
/* Report the filesystem types sg_fs_stats() returns under the default
 * filter, so every set the race installs keeps at least one mount visible. */
$fs = sg_fs_stats();
$types = [];
foreach (is_array($fs) ? $fs : [] as $row) {
    if (preg_match('/^[A-Za-z0-9._-]+$/', $row['fs_type'])) {
        $types[$row['fs_type']] = true;
    }
}
echo 'types: ', implode(',', array_keys($types)), "\n";
