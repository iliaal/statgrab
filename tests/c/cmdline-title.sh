#!/bin/sh
# Linux regression test for libstatgrab's proctitle rewrite in
# sg_get_process_stats(). LIBSTATGRAB_DIR names a configured and built
# libstatgrab tree (default: the in-tree vendor/libstatgrab). It must be built
# with -fsanitize=address,undefined: the out-of-bounds heap access is only
# reported reliably under ASan.
set -eu
src=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
lib="${LIBSTATGRAB_DIR:-$src/vendor/libstatgrab}/src/libstatgrab"
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT HUP INT TERM
if test "$(uname -s)" != Linux; then
    echo 'SKIP: /proc/PID/cmdline interposition requires Linux'
    exit 0
fi
if test ! -r "$lib/.libs/libstatgrab.a" || test ! -r "$lib/statgrab.h"; then
    echo "FAIL: no built libstatgrab under $lib (see vendor/libstatgrab/LOCAL_PATCHES.md)" >&2
    exit 1
fi
if ! nm "$lib/.libs/libstatgrab.a" 2>/dev/null | grep -q __asan_report; then
    echo "FAIL: $lib/.libs/libstatgrab.a is not ASan-instrumented; rebuild it with" >&2
    echo '  CFLAGS="-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer" LDFLAGS="-fsanitize=address,undefined"' >&2
    exit 1
fi
"${CC:-cc}" -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I"$lib" "$src/tests/c/cmdline-title.c" -o "$work/cmdline-title" \
    -Wl,--wrap=open -Wl,--wrap=sg_realloc "$lib/.libs/libstatgrab.a" -lpthread
ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=1} \
UBSAN_OPTIONS="${UBSAN_OPTIONS:+$UBSAN_OPTIONS:}halt_on_error=1" \
    "$work/cmdline-title"
