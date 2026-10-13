--TEST--
sg_process_stats: session_id is zero on Linux, where libstatgrab does not parse it
--EXTENSIONS--
statgrab
--SKIPIF--
<?php
if (PHP_OS_FAMILY !== 'Linux') die('skip Linux /proc backend only');
/* Only the bundled, statically linked copy carries the sessid fix; a shared
 * system libstatgrab 0.92.1 still returns uninitialised heap bytes. */
if (preg_match('~/libstatgrab\.so~', (string) @file_get_contents('/proc/self/maps'))) {
    die('skip needs bundled libstatgrab (vendor/libstatgrab/patches/0003)');
}
?>
--FILE--
<?php
/* The Linux backend never reads /proc/PID/stat field 6, so every row must
 * carry the initialised value rather than leftover heap bytes. Several calls
 * let the vector reuse memory a previous call already filled. */
$bad = [];
for ($round = 0; $round < 3; $round++) {
    foreach ([sg_process_stats(), (new Statgrab())->processes()] as $rows) {
        foreach ($rows as $r) {
            if ($r['session_id'] !== 0) {
                $bad[$r['pid']] = $r['session_id'];
            }
        }
    }
}
if ($bad) {
    foreach (array_slice($bad, 0, 5, true) as $pid => $sid) {
        echo "FAIL: pid $pid session_id $sid\n";
    }
}
echo "DONE\n";
?>
--EXPECT--
DONE
