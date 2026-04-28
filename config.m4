dnl config.m4 for extension statgrab

PHP_ARG_WITH([statgrab], [for libstatgrab support],
[AS_HELP_STRING([[--with-statgrab[=DIR|=bundled]]],
  [Include libstatgrab >= 0.92 support. =bundled uses the patched copy
   under vendor/libstatgrab/ (must be built first; see vendor/libstatgrab/LOCAL_PATCHES.md).
   Default: detect via pkg-config / standard prefixes.])])

if test "$PHP_STATGRAB" != "no"; then
  STATGRAB_FOUND=no

  dnl --- bundled: vendored + patched libstatgrab built under vendor/libstatgrab/ ---
  if test "$PHP_STATGRAB" = "bundled"; then
    AC_MSG_CHECKING([for bundled libstatgrab])
    BUNDLED_INC="$abs_srcdir/vendor/libstatgrab/src/libstatgrab"
    BUNDLED_LIB="$abs_srcdir/vendor/libstatgrab/src/libstatgrab/.libs/libstatgrab.a"
    if test ! -r "$BUNDLED_INC/statgrab.h"; then
      AC_MSG_RESULT([statgrab.h not found])
      AC_MSG_ERROR([Bundled libstatgrab not configured. Run:
                      (cd vendor/libstatgrab && ./configure --enable-static --disable-shared --without-ncurses && make)
                    then re-run ./configure --with-statgrab=bundled.])
    fi
    if test ! -r "$BUNDLED_LIB"; then
      AC_MSG_RESULT([libstatgrab.a not built])
      AC_MSG_ERROR([Bundled libstatgrab not built. Run 'make -C vendor/libstatgrab' first.])
    fi
    AC_MSG_RESULT([using $BUNDLED_LIB])
    PHP_ADD_INCLUDE([$BUNDLED_INC])
    PHP_ADD_LIBRARY_WITH_PATH([statgrab], [$abs_srcdir/vendor/libstatgrab/src/libstatgrab/.libs], [STATGRAB_SHARED_LIBADD])
    STATGRAB_FOUND=yes
  fi

  dnl --- pkg-config detection ---
  if test "$STATGRAB_FOUND" = "no"; then
    AC_PATH_PROG([PKG_CONFIG], [pkg-config], [no])
    if test "x$PKG_CONFIG" != "xno"; then
      if test "$PHP_STATGRAB" = "yes" || test -z "$PHP_STATGRAB"; then
        if $PKG_CONFIG --atleast-version=0.92 libstatgrab 2>/dev/null; then
          STATGRAB_VERSION=`$PKG_CONFIG --modversion libstatgrab`
          STATGRAB_INCS=`$PKG_CONFIG --cflags libstatgrab`
          STATGRAB_LIBSDESC=`$PKG_CONFIG --libs libstatgrab`
          AC_MSG_CHECKING([for libstatgrab via pkg-config])
          AC_MSG_RESULT([$STATGRAB_VERSION])
          PHP_EVAL_INCLINE([$STATGRAB_INCS])
          PHP_EVAL_LIBLINE([$STATGRAB_LIBSDESC], [STATGRAB_SHARED_LIBADD])
          STATGRAB_FOUND=yes
        fi
      fi
    fi
  fi

  dnl --- explicit-path / default-path probe ---
  if test "$STATGRAB_FOUND" = "no"; then
    if test "$PHP_STATGRAB" = "yes" || test -z "$PHP_STATGRAB"; then
      SEARCH_PATH="/usr/local /usr"
    else
      SEARCH_PATH="$PHP_STATGRAB"
    fi
    STATGRAB_DIR=
    AC_MSG_CHECKING([for libstatgrab in default paths])
    for i in $SEARCH_PATH ; do
      if test -r "$i/include/statgrab.h"; then
        STATGRAB_DIR=$i
        AC_MSG_RESULT([found in $i])
        break
      fi
    done
    if test -z "$STATGRAB_DIR"; then
      AC_MSG_RESULT([not found])
      AC_MSG_ERROR([libstatgrab >= 0.92 not found. Install libstatgrab-dev (Debian/Ubuntu),
                    libstatgrab-devel (RHEL/Fedora), libstatgrab (Homebrew/FreeBSD pkg);
                    pass --with-statgrab=<prefix> for a custom install; or
                    --with-statgrab=bundled to use vendor/libstatgrab/ (build it first).])
    fi
    PHP_ADD_INCLUDE([$STATGRAB_DIR/include])
    PHP_ADD_LIBRARY_WITH_PATH([statgrab], [$STATGRAB_DIR/$PHP_LIBDIR], [STATGRAB_SHARED_LIBADD])
  fi

  PHP_SUBST([STATGRAB_SHARED_LIBADD])
  AC_DEFINE([HAVE_LIBSTATGRAB], 1, [Have libstatgrab >= 0.92])
  PHP_NEW_EXTENSION([statgrab], [statgrab.c], [$ext_shared])
fi
