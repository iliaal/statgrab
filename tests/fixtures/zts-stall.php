<?php
/* Thread 0 starts a filesystem walk that stalls on one mount; threads 1 and
 * 2 start while it is stalled and must not queue behind it. */
$id = $GLOBALS['SG_THREAD'];
$t0 = (float)getenv('SG_T0');
$wait = $t0 + ($id === 0 ? 0 : 0.3) - microtime(true);
if ($wait > 0) {
    usleep((int)($wait * 1e6));
}
$start = microtime(true);
$ok = $id === 2 ? is_array(sg_valid_filesystems()) : is_array(sg_fs_stats());
$end = microtime(true);
printf("%s %d %s end=%.3f dur=%.3f\n", $ok ? 'ok' : 'FAIL', $id, $id === 2 ? 'list' : 'fs', $end - $t0, $end - $start);
