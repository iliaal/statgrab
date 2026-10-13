#!/bin/sh
# Run after a --with-statgrab=bundled build: sh tests/bundled-static.sh [path/to/statgrab.so]
set -eu

src=$(CDPATH='' cd -- "$(dirname -- "$0")/.." && pwd)
so=${1:-$src/modules/statgrab.so}

if test ! -r "$so"; then
    echo "FAIL: $so not found" >&2
    exit 1
fi

deps=$(ldd "$so")
printf '%s\n' "$deps"
if printf '%s\n' "$deps" | grep -i libstatgrab >/dev/null; then
    echo "FAIL: $so depends on a shared libstatgrab" >&2
    exit 1
fi
echo 'PASS: no shared libstatgrab dependency'

if ! nm -D --defined-only "$so" | grep -w sg_init >/dev/null; then
    echo "FAIL: $so does not define sg_init; bundled libstatgrab not linked in" >&2
    exit 1
fi
echo 'PASS: bundled libstatgrab linked into the module'
