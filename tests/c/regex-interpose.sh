#!/bin/sh
# Linux regression test: libstatgrab keeps using glibc's regex when the
# program exports its own regcomp()/regexec()/regfree(). LIBSTATGRAB_DIR
# names a configured and built libstatgrab tree (default: the in-tree vendor/libstatgrab), built with
# -fsanitize=address,undefined like the other tests in this directory.
set -eu
src=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
lib="${LIBSTATGRAB_DIR:-$src/vendor/libstatgrab}/src/libstatgrab"
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT HUP INT TERM
if test "$(uname -s)" != Linux; then
    echo 'SKIP: /proc/net/dev parsing requires Linux'
    exit 0
fi
if test ! -r "$lib/.libs/libstatgrab.a" || test ! -r "$lib/statgrab.h"; then
    echo "FAIL: no built libstatgrab under $lib (see vendor/libstatgrab/LOCAL_PATCHES.md)" >&2
    exit 1
fi
# Reject failed/partial inspection before checking instrumentation.
symbols=$(nm "$lib/.libs/libstatgrab.a")
if ! printf '%s\n' "$symbols" | grep __asan_report >/dev/null; then
    echo "FAIL: $lib/.libs/libstatgrab.a is not ASan-instrumented; rebuild it with" >&2
    echo '  CFLAGS="-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer" LDFLAGS="-fsanitize=address,undefined"' >&2
    exit 1
fi
"${CC:-cc}" -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I"$lib" "$src/tests/c/regex-interpose.c" -o "$work/regex-interpose" \
    "$lib/.libs/libstatgrab.a" -lpthread -ldl
ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=1} \
UBSAN_OPTIONS="${UBSAN_OPTIONS:+$UBSAN_OPTIONS:}halt_on_error=1" \
    "$work/regex-interpose"
