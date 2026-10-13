#!/bin/sh
# Linux regression test for the bundled libstatgrab's sg_vector_compute_diff().
# Requires vendor/libstatgrab built with -fsanitize=address: the overflow is a
# stack read/write that only ASan reports reliably.
set -eu
src=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
lib="$src/vendor/libstatgrab/src/libstatgrab"
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT HUP INT TERM
if test "$(uname -s)" != Linux; then
    echo 'SKIP: /proc/net/dev interposition requires Linux'
    exit 0
fi
if test ! -r "$lib/.libs/libstatgrab.a" || test ! -r "$lib/statgrab.h"; then
    echo "FAIL: build vendor/libstatgrab first (see vendor/libstatgrab/LOCAL_PATCHES.md)" >&2
    exit 1
fi
if ! nm "$lib/.libs/libstatgrab.a" 2>/dev/null | grep -q __asan_report; then
    echo 'FAIL: vendor/libstatgrab is not ASan-instrumented; rebuild it with' >&2
    echo '  CFLAGS="-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer" LDFLAGS="-fsanitize=address,undefined"' >&2
    exit 1
fi
"${CC:-cc}" -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I"$lib" "$src/tests/c/diff-shrink.c" -o "$work/diff-shrink" \
    -Wl,--wrap=fopen "$lib/.libs/libstatgrab.a" -lpthread
ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=1} "$work/diff-shrink"
