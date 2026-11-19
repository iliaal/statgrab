#!/bin/sh
# ZTS integration test: a filesystem walk stalled on one slow mount must not
# serialize other threads' sg_fs_stats() / sg_valid_filesystems() calls.
# usage: tests/zts-fs-stall.sh <php-config>
# <php-config> must belong to a ZTS PHP built with --enable-embed, and
# modules/statgrab.so must be built against it.
set -eu
src=$(CDPATH='' cd -- "$(dirname -- "$0")/.." && pwd)
fixtures=$src/tests/fixtures
ext=$src/modules/statgrab.so

if test $# -ne 1; then
    echo "usage: $0 <zts-embed-php-config>" >&2
    exit 2
fi
php_config=$1
if ! "$php_config" --php-sapis | grep -w embed >/dev/null; then
    echo "$php_config: PHP was not built with --enable-embed" >&2
    exit 2
fi
if test ! -r "$ext"; then
    echo "$ext: build the extension first" >&2
    exit 2
fi

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT HUP INT TERM
libdir=$("$php_config" --prefix)/lib

san=
symbols=$(nm -D --undefined-only "$libdir/libphp.so")
if printf '%s\n' "$symbols" | grep __asan_init >/dev/null; then
    san='-fsanitize=address,undefined -fno-omit-frame-pointer'
fi
# shellcheck disable=SC2046,SC2086
"${CC:-cc}" -g -O1 $san $("$php_config" --includes) \
    "$fixtures/zts-driver.c" -o "$work/driver" \
    -L"$libdir" -Wl,-rpath,"$libdir" -lphp -pthread
"${CC:-cc}" -g -O1 -fPIC -shared "$fixtures/slow-statvfs.c" -o "$work/slow-statvfs.so" -ldl

ASAN_OPTIONS=detect_leaks=0:halt_on_error=1:detect_odr_violation=0:verify_asan_link_order=0
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1
export ASAN_OPTIONS UBSAN_OPTIONS

# "/" is reported on every supported host and passes the default filter.
SG_SLOW_PATH=/ SG_SLOW_MS=2000 SG_T0=$(awk 'BEGIN { srand(); printf "%.3f", srand() + 2 }') \
LD_PRELOAD=$work/slow-statvfs.so \
    timeout 120 "$work/driver" "$ext" 1 0 \
    "$fixtures/zts-stall.php" "$fixtures/zts-stall.php" "$fixtures/zts-stall.php" \
    >"$work/log" 2>&1 || { echo 'FAIL: driver'; cat "$work/log"; exit 1; }
cat "$work/log"

hits=$(sed -n 's/^slow statvfs hits: //p' "$work/log")
if test "${hits:-0}" -lt 2; then
    echo "FAIL: the slow mount was walked ${hits:-0} times, need 2"
    exit 1
fi
# Serialized, walk 1 ends about 2 s after walk 0 and the list call waits
# about 1.7 s; concurrent, both differences stay near 0.3 s and 0 s.
awk '
    $1 == "FAIL" { bad = 1 }
    $1 != "ok" { next }
    $2 == 0 { split($4, a, "="); end0 = a[2] }
    $2 == 1 { split($4, a, "="); end1 = a[2] }
    $2 == 2 { split($5, a, "="); list = a[2] }
    END {
        if (bad || end0 == "" || end1 == "" || list == "") { print "FAIL: missing or failed probe"; exit 1 }
        if (end1 - end0 > 1.0) { printf "FAIL: second walk queued %.3f s behind the stalled one\n", end1 - end0; exit 1 }
        if (list > 1.0) { printf "FAIL: sg_valid_filesystems() waited %.3f s for the stalled walk\n", list; exit 1 }
        print "PASS"
    }' "$work/log"
