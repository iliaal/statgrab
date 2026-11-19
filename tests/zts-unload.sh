#!/bin/sh
# ZTS integration test: a thread that used statgrab exits after PHP has
# unloaded shared extensions, then PHP starts again in the same process.
# Then a host dlopen()s libphp and unloads it again, and neither libphp nor
# statgrab.so may stay mapped.
# usage: tests/zts-unload.sh <php-config>
# <php-config> must belong to a ZTS PHP built with --enable-embed, and
# modules/statgrab.so must be built against it.
set -eu
src=$(CDPATH='' cd -- "$(dirname -- "$0")/.." && pwd)
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
    "$src/tests/fixtures/zts-unload.c" -o "$work/driver" \
    -L"$libdir" -Wl,-rpath,"$libdir" -lphp -pthread
# shellcheck disable=SC2086
"${CC:-cc}" -g -O1 $san "$src/tests/fixtures/embed-dlclose.c" -o "$work/dlclose" -ldl

ASAN_OPTIONS=detect_leaks=0:halt_on_error=1:detect_odr_violation=0
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1
export ASAN_OPTIONS UBSAN_OPTIONS

status=0
timeout 120 "$work/driver" "$ext" 3 >"$work/log" 2>&1 || status=$?
got=$(grep -c '^ok ' "$work/log" || true)
if test "$status" -ne 0 || test "$got" -ne 6; then
    echo "FAIL (exit $status, $got/6 ok lines)"
    cat "$work/log"
    exit 1
fi
echo "PASS threads ($got/6 ok lines)"

status=0
timeout 120 "$work/dlclose" "$libdir/libphp.so" "$ext" >"$work/dlclose.log" 2>&1 || status=$?
got=$(grep -c '^ok ' "$work/dlclose.log" || true)
if test "$status" -ne 0 || test "$got" -ne 4; then
    echo "FAIL dlclose (exit $status, $got/4 ok lines)"
    cat "$work/dlclose.log"
    exit 1
fi
echo "PASS dlclose ($got/4 ok lines)"
