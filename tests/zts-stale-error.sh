#!/bin/sh
# ZTS integration test: an error left by one request must not show up in
# sg_error_details() of the next request served on the same thread, including
# the next HTTP request of a FrankenPHP-style worker loop.
# usage: tests/zts-stale-error.sh <php-config>
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
"${CC:-cc}" -g -O1 -fPIC -shared "$fixtures/fail-meminfo.c" -o "$work/fail-meminfo.so" -ldl

ASAN_OPTIONS=detect_leaks=0:halt_on_error=1:detect_odr_violation=0:verify_asan_link_order=0
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1
export ASAN_OPTIONS UBSAN_OPTIONS

# classic: two PHP requests on one thread. worker: one PHP request carrying
# two HTTP requests, separated the way FrankenPHP worker mode does it.
# worker-between: the error comes from worker code that runs between two
# HTTP requests, after the first one's sapi_deactivate().
rc=0
for leg in classic worker worker-between; do
    between=
    case $leg in
        classic) requests=2 cycles=1 ;;
        worker) requests=1 cycles=2 ;;
        worker-between) requests=1 cycles=2 between=1 ;;
    esac
    status=0
    if test -n "$between"; then SG_WORKER_BETWEEN=1; export SG_WORKER_BETWEEN; else unset SG_WORKER_BETWEEN; fi
    SG_WORKER_CYCLES=$cycles LD_PRELOAD=$work/fail-meminfo.so timeout 120 \
        "$work/driver" "$ext" "$requests" 0 "$fixtures/zts-stale-error.php" >"$work/$leg.log" 2>&1 || status=$?
    if test "$status" -ne 0 || ! grep -x 'ok failed' "$work/$leg.log" >/dev/null \
        || ! grep -x 'ok fresh' "$work/$leg.log" >/dev/null \
        || grep '^FAIL' "$work/$leg.log" >/dev/null; then
        echo "FAIL $leg (exit $status)"
        cat "$work/$leg.log"
        rc=1
    else
        echo "PASS $leg"
    fi
done
exit $rc
