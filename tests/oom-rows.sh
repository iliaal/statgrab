#!/bin/sh
# Memory-limit regression test: a stat call whose result array outgrows
# memory_limit must end with PHP's "Allowed memory size" fatal error. PHP 8.5+
# queues that error instead of bailing out of the allocation, and kills the
# process with exit status 137 at twice memory_limit if the row loop keeps
# going. Rows come from tests/fixtures/many-rows.c (LD_PRELOAD) and, for
# processes, from short-lived child processes.
# usage: tests/oom-rows.sh <php>
# modules/statgrab.so must be built against <php>.
set -eu
src=$(CDPATH='' cd -- "$(dirname -- "$0")/.." && pwd)
ext=$src/modules/statgrab.so

if test $# -ne 1; then
    echo "usage: $0 <php>" >&2
    exit 2
fi
php=$1
if test "$(uname -s)" != Linux; then
    echo 'SKIP: /proc and utmp interposition requires Linux'
    exit 0
fi
if test ! -r "$ext"; then
    echo "$ext: build the extension first" >&2
    exit 2
fi

work=$(mktemp -d)
pids=
cleanup() {
    # shellcheck disable=SC2086
    test -z "$pids" || kill $pids 2>/dev/null || true
    rm -rf "$work"
}
trap cleanup EXIT HUP INT TERM
"${CC:-cc}" -g -O1 -fPIC -shared "$src/tests/fixtures/many-rows.c" -o "$work/many-rows.so" -ldl

# The limit sits just above the memory already in use, so the result array
# alone crosses it and, without the check, then reaches twice the limit.
cat >"$work/probe.php" <<'EOF'
<?php
$fn = $argv[1];
if ($fn === 'sg_fs_stats') {
    sg_set_valid_filesystems(['ext4']);
}
ini_set('memory_limit', (string)(memory_get_usage(true) + 64 * 1024));
$r = $fn();
echo "FAIL: $fn returned ", is_array($r) ? count($r) . ' rows' : var_export($r, true), "\n";
EOF

rc=0
probe() {
    status=0
    "$@" >"$work/log" 2>&1 || status=$?
    if test "$status" -ne 137 && grep 'Allowed memory size of [0-9]* bytes exhausted' "$work/log" >/dev/null; then
        echo "PASS $fn (exit $status)"
    else
        echo "FAIL $fn (exit $status)"
        cat "$work/log"
        rc=1
    fi
}

# PHP builds that load extensions with RTLD_DEEPBIND bind statgrab.so to
# libc before the preloaded shim, so the shim cannot add rows there.
seen=$(SG_ROWS=3 LD_PRELOAD="$work/many-rows.so" "$php" -n -d extension="$ext" \
    -r 'echo count(sg_diskio_stats() ?: []);')
if test "$seen" = 3; then
    for fn in sg_fs_stats sg_network_stats sg_network_iface_stats sg_diskio_stats sg_user_stats; do
        probe env SG_ROWS=40000 LD_PRELOAD="$work/many-rows.so" \
            "$php" -n -d extension="$ext" "$work/probe.php" "$fn"
    done
elif "$php" -n -r 'exit(PHP_VERSION_ID >= 80500 ? 0 : 1);'; then
    # PHP 8.5+ is where the row loops can reach the hard limit, so the
    # shim-driven calls must run there.
    echo "FAIL: LD_PRELOAD does not reach statgrab.so (got $seen rows, want 3)"
    rc=1
else
    echo "SKIP shim-driven calls: LD_PRELOAD does not reach statgrab.so (got $seen rows, want 3)"
fi

i=0
while test "$i" -lt 4000; do
    sleep 120 &
    pids="$pids $!"
    i=$((i + 1))
done
fn=sg_process_stats
probe "$php" -n -d extension="$ext" "$work/probe.php" "$fn"
exit $rc
