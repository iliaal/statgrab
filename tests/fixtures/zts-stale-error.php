<?php
/* One step leaves a libstatgrab error on this thread: the first request, or
 * with SG_WORKER_BETWEEN the worker code between two HTTP requests. Every
 * later request on the same thread must not see it as its own. */
$between = getenv('SG_WORKER_BETWEEN') !== false;
$fails = $between ? $GLOBALS['SG_BETWEEN'] === 1 : $GLOBALS['SG_REQUEST'] + $GLOBALS['SG_CYCLE'] === 0;
if ($fails) {
    $details = @sg_memory_stats() === false ? sg_error_details() : false;
    echo is_array($details) && $details['code'] === Statgrab::ERROR_OPEN ? "ok failed\n" : "FAIL failed: " . json_encode($details) . "\n";
} else {
    $details = sg_error_details();
    echo $details === false ? "ok fresh\n" : "FAIL fresh: " . json_encode($details) . "\n";
}
