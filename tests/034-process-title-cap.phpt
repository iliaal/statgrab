--TEST--
sg_process_stats() caps proc_title at 4096 bytes on a UTF-8 boundary
--EXTENSIONS--
statgrab
--SKIPIF--
<?php
if (PHP_OS_FAMILY !== 'Linux') die('skip needs /proc/<pid>/cmdline');
if (!function_exists('proc_open')) die('skip proc_open() is disabled');
if (!is_executable('/bin/sh')) die('skip needs /bin/sh');
$null = ['file', '/dev/null', 'w'];
$p = proc_open(['/bin/sh', '-c', 'exit 0', 'sh', str_repeat('a', 8192)], [1 => $null, 2 => $null], $pipes);
if (!is_resource($p) || proc_close($p) !== 0) die('skip cannot spawn a process with an 8 KiB argument');
?>
--INI--
memory_limit=128M
--FILE--
<?php
const TITLE_MAX = 4096;
const MAX_ARGS = 8;
const MAX_ARG_BYTES = 120000; /* Linux caps one argv string at 128 KiB. */
const MIN_ARG_BYTES = 8192;
const CHILDREN_PER_CASE = 2;

/* Every case puts the cap on the last byte of a character, so the title
 * must keep exactly 4097 - width bytes: all of them for ASCII, and the
 * characters before the split one for multibyte UTF-8. */
$cases = ['ascii' => 'a', 'utf8-2' => "\u{e9}", 'utf8-3' => "\u{20ac}", 'utf8-4' => "\u{1f600}"];

$children = [];
register_shutdown_function(function () use (&$children) {
    foreach ($children as $child) {
        foreach ($child['pipes'] as $pipe) {
            if (is_resource($pipe)) {
                fclose($pipe);
            }
        }
        proc_terminate($child['proc'], 9);
        proc_close($child['proc']);
    }
});

function build_argv(string $char, int $args, int $arg_bytes): array
{
    $width = strlen($char);
    $marker = 'statgrab-title-cap';
    while ((TITLE_MAX - strlen("/bin/sh -c read _ $marker ")) % $width !== $width - 1) {
        $marker .= 'x';
    }
    $arg = str_repeat($char, intdiv($arg_bytes, $width));
    return array_merge(['/bin/sh', '-c', 'read _', $marker], array_fill(0, $args, $arg));
}

/* "read" blocks on the stdin pipe, so the shell also exits if this test
 * dies. Returns null when exec fails, e.g. with E2BIG under a low ARG_MAX. */
function spawn(array $argv): ?array
{
    $null = ['file', '/dev/null', 'w'];
    $proc = @proc_open($argv, [0 => ['pipe', 'r'], 1 => $null, 2 => $null], $pipes);
    if (!is_resource($proc)) {
        return null;
    }
    $pid = proc_get_status($proc)['pid'];
    $want = strlen(implode("\0", $argv)) + 1;
    $deadline = microtime(true) + 10;
    while (microtime(true) < $deadline && proc_get_status($proc)['running']) {
        if (strlen((string)@file_get_contents("/proc/$pid/cmdline")) === $want) {
            return ['proc' => $proc, 'pipes' => $pipes, 'pid' => $pid, 'want' => implode(' ', $argv)];
        }
        usleep(10000);
    }
    foreach ($pipes as $pipe) {
        fclose($pipe);
    }
    proc_terminate($proc, 9);
    proc_close($proc);
    return null;
}

/* Measure the result size with ordinary titles; the limit set below leaves
 * room for that plus slack, but not for eight uncapped ~1 MB titles. */
$before = memory_get_usage();
$rows = sg_process_stats();
$baseline = memory_get_usage() - $before;
unset($rows);

/* Use the largest argv this host can exec; any size above the cap still
 * exercises it, but only the largest also stresses memory_limit. */
$size = null;
for ($bytes = MAX_ARG_BYTES; $size === null && $bytes >= MIN_ARG_BYTES; $bytes = intdiv($bytes, 2)) {
    for ($args = MAX_ARGS; $args >= 1; $args = intdiv($args, 2)) {
        $child = spawn(build_argv('a', $args, $bytes));
        if ($child !== null) {
            $child['case'] = 'ascii';
            $children[] = $child;
            $size = [$args, $bytes];
            break;
        }
    }
}
if ($size === null) {
    echo "FAIL: no oversized argv could be spawned\n";
    exit(1);
}

foreach ($cases as $case => $char) {
    $have = count(array_filter($children, fn($c) => $c['case'] === $case));
    for ($i = $have; $i < CHILDREN_PER_CASE; $i++) {
        $child = spawn(build_argv($char, ...$size));
        if ($child === null) {
            echo "FAIL: spawn $case\n";
            exit(1);
        }
        $child['case'] = $case;
        $children[] = $child;
    }
}

ini_set('memory_limit', (string)(memory_get_usage(true) + (int)(1.5 * $baseline) + (4 << 20)));

$ok = array_fill_keys(array_keys($cases), 0);
foreach (sg_process_stats() as $row) {
    foreach ($children as $child) {
        if ($row['pid'] !== $child['pid']) {
            continue;
        }
        $title = $row['proc_title'];
        $len = TITLE_MAX + 1 - strlen($cases[$child['case']]);
        if (strlen($title) !== $len) {
            echo "FAIL: {$child['case']} title is ", strlen($title), " bytes, want $len\n";
        } elseif (!str_starts_with($child['want'], $title)) {
            echo "FAIL: {$child['case']} title is not a prefix of its argv\n";
        } elseif (preg_match('//u', $title) !== 1) {
            echo "FAIL: {$child['case']} title split a UTF-8 sequence\n";
        } else {
            $ok[$child['case']]++;
        }
    }
}
foreach ($ok as $case => $n) {
    echo "$case: $n ok\n";
}
?>
--EXPECT--
ascii: 2 ok
utf8-2: 2 ok
utf8-3: 2 ok
utf8-4: 2 ok
