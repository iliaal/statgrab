#!/bin/sh
# Run with PHPIZE=phpize PHP_CONFIG=php-config sh tests/configure-prefix.sh.
# Tiny fixtures keep these configure-only checks independent of system libraries.
set -eu

src=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT HUP INT TERM
mkdir -p "$work/build" "$work/prefix/include" "$work/prefix/lib"
cp "$src/config.m4" "$src/statgrab.c" "$work/build/"
cd "$work/build"
"${PHPIZE:-phpize}" > "$work/phpize.log" 2>&1 || {
    cat "$work/phpize.log"
    exit 1
}

cat > "$work/prefix/include/statgrab.h" <<'EOF'
typedef struct { int error; } sg_error_details;
int sg_get_error_details(sg_error_details *details);
EOF
printf '%s\n' 'int sg_init(int ignore_errors) { return ignore_errors; }' > "$work/stub.c"
"${CC:-cc}" -c "$work/stub.c" -o "$work/stub.o"
"${AR:-ar}" cr "$work/prefix/lib/libstatgrab.a" "$work/stub.o"

check_configure() {
    expected=$1
    libdir=$2
    message=$3
    if CPPFLAGS= LDFLAGS= LIBS= ./configure \
        --with-php-config="${PHP_CONFIG:-php-config}" \
        --with-statgrab="$work/prefix" --with-libdir="$libdir" \
        > "$work/configure.log" 2>&1; then
        actual=success
    else
        actual=failure
    fi
    if test "$actual" != "$expected" || ! grep -F "$message" "$work/configure.log" >/dev/null; then
        cat "$work/configure.log"
        echo "Expected $expected: $message" >&2
        exit 1
    fi
}

check_configure success lib 'checking for sg_get_error_details in libstatgrab... yes'
echo 'PASS: custom prefix without caller-supplied compiler/linker flags'
mv "$work/prefix/lib" "$work/prefix/lib64"
check_configure success lib64 'checking for sg_get_error_details in libstatgrab... yes'
echo 'PASS: custom library directory'

printf '%s\n' '/* Old header without the required API. */' > "$work/prefix/include/statgrab.h"
check_configure failure lib64 'lacks sg_get_error_details; need >= 0.92'
echo 'PASS: incompatible header rejected'

rm "$work/prefix/lib64/libstatgrab.a"
printf '%s\n' 'int unrelated(void) { return 0; }' > "$work/stub.c"
"${CC:-cc}" -c "$work/stub.c" -o "$work/stub.o"
"${AR:-ar}" cr "$work/prefix/lib64/libstatgrab.a" "$work/stub.o"
check_configure failure lib64 'libstatgrab link check failed'
echo 'PASS: incompatible library rejected'
