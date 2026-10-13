#!/bin/sh
# ZTS integration test: races the process-global valid-filesystems list from
# several PHP threads in one embed-SAPI process.
# usage: tests/zts-valid-filesystems.sh <php-config> [seconds]
# <php-config> must belong to a ZTS PHP built with --enable-embed, and
# modules/statgrab.so must be built against it. On an ASan PHP the driver is
# built with ASan as well, and any sanitizer error fails the run. Without
# ASan the run only warns, unless $CI is set, where it fails.
set -eu
src=$(CDPATH='' cd -- "$(dirname -- "$0")/.." && pwd)
fixtures=$src/tests/fixtures
ext=$src/modules/statgrab.so

if test $# -lt 1 || test $# -gt 2; then
    echo "usage: $0 <zts-embed-php-config> [seconds]" >&2
    exit 2
fi
php_config=$1
seconds=${2:-5}
case $seconds in
    ''|*[!0-9]*) echo 'seconds must be a non-negative integer' >&2; exit 2 ;;
esac
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
if nm -D --undefined-only "$libdir/libphp.so" | grep __asan_init >/dev/null; then
    san='-fsanitize=address,undefined -fno-omit-frame-pointer'
else
    echo "WARNING: $libdir/libphp.so has no ASan runtime; races may pass undetected" >&2
    if test -n "${CI:-}"; then
        echo 'FAIL: CI must run this test against an ASan PHP' >&2
        exit 1
    fi
fi
# Word splitting is intentional for the sanitizer and include flags.
# shellcheck disable=SC2046,SC2086
"${CC:-cc}" -g -O1 $san $("$php_config" --includes) \
    "$fixtures/zts-driver.c" -o "$work/driver" \
    -L"$libdir" -Wl,-rpath,"$libdir" -lphp -pthread

# Leaks are covered by the NTS ASan suite; this run checks memory safety.
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1:detect_odr_violation=0
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1
export ASAN_OPTIONS UBSAN_OPTIONS

"$work/driver" "$ext" 1 0 "$fixtures/zts-types.php" >"$work/types.log" 2>&1 || {
    echo 'FAIL: filesystem type probe'
    cat "$work/types.log"
    exit 1
}
SG_ZTS_FS_TYPES=$(sed -n 's/^types: //p' "$work/types.log")
if test -z "$SG_ZTS_FS_TYPES"; then
    if test -n "${CI:-}"; then
        echo 'FAIL: sg_fs_stats() reports no usable filesystem types; CI must exercise the race'
        exit 1
    fi
    echo 'SKIP: sg_fs_stats() reports no usable filesystem types on this host'
    exit 0
fi
export SG_ZTS_FS_TYPES
echo "filesystem types: $SG_ZTS_FS_TYPES"

failed=0

# scenario <name> <requests per thread> <expected ok lines> <script>...
scenario() {
    name=$1 requests=$2 want=$3
    shift 3
    log=$work/$name.log
    status=0
    timeout $((seconds + 300)) \
        "$work/driver" "$ext" "$requests" "$seconds" "$@" >"$log" 2>&1 || status=$?
    got=$(grep -o 'ok [a-z]*' "$log" | wc -l)
    if test "$status" -ne 0 || test "$got" -ne "$want" \
        || grep -e 'ERROR: AddressSanitizer' -e 'runtime error:' -e 'FAIL [a-z]*:' "$log" >/dev/null; then
        echo "FAIL $name (exit $status, $got/$want ok lines)"
        cat "$log"
        failed=1
    else
        echo "PASS $name ($got/$want ok lines)" \
            "$(grep -o 'ok [a-z]* [0-9][0-9]*' "$log" | tr '\n' ' ')"
    fi
}

scenario setter-vs-fs 1 2 "$fixtures/zts-setter.php" "$fixtures/zts-fs.php"
scenario setter-vs-list 1 2 "$fixtures/zts-setter.php" "$fixtures/zts-list.php"
scenario symmetric 200 800 "$fixtures/zts-symmetric.php" "$fixtures/zts-symmetric.php" \
    "$fixtures/zts-symmetric.php" "$fixtures/zts-symmetric.php"

exit $failed
