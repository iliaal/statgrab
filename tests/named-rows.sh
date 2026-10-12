#!/bin/sh
# Linux integration test; build the extension against system libstatgrab first.
# The fixture supplies deterministic stats using PHP's real array functions.
set -eu
src=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT HUP INT TERM
php=${TEST_PHP_EXECUTABLE:-php}
if test "$(uname -s)" != Linux; then
    echo 'SKIP: named-row interposition requires Linux'
    exit 0
fi
int_size=$("$php" -n -r 'echo PHP_INT_SIZE;')
if test "$int_size" != 8; then
    echo 'SKIP: named-row interposition requires 64-bit PHP'
    exit 0
fi
# Word splitting is intentional for compiler flags supplied by pkg-config.
"${CC:-cc}" -shared -fPIC $(pkg-config --cflags libstatgrab) \
    "$src/tests/fixtures/named-rows.c" -o "$work/named-rows.so" -ldl
LD_PRELOAD="$work/named-rows.so${LD_PRELOAD:+:$LD_PRELOAD}" \
    "$php" -n -d "extension=$src/modules/statgrab.so" "$src/tests/fixtures/named-rows.php"
