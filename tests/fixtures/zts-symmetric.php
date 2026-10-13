<?php
/* Every thread sets its filter and reads filesystems in each request. */
$sg = new Statgrab();
$ok = $sg->setValidFilesystems(explode(',', getenv('SG_ZTS_FS_TYPES')))
    && is_array(sg_fs_stats())
    && is_array($sg->validFilesystems());
if (!$ok) {
    echo "FAIL symmetric: thread {$GLOBALS['SG_THREAD']} request {$GLOBALS['SG_REQUEST']}\n";
    return;
}
echo "ok symmetric\n";
