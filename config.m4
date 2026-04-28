dnl $Id: config.m4,v 1.10 2006/03/23 18:43:49 iliaa Exp $
dnl config.m4 for extension statgrab

PHP_ARG_WITH(statgrab, for libstatgrab support,
[  --with-statgrab             Include libstatgrab support])

AC_DEFUN([PHP_STATGRAB_CHECK_VERSION],[
  PHP_CHECK_LIBRARY(statgrab, get_network_iface_stats,        [AC_DEFINE(HAVE_STATGRAB_09,             1, [ ])], [], [ -L$STATGRAB_DIR/lib $STATGRAB_SHARED_LIBADD ])
])

if test "$PHP_STATGRAB" != "no"; then
  SEARCH_PATH="/usr/local /usr $PHP_STATGRAB"
  SEARCH_FOR="/include/statgrab.h"
  if test -r $PHP_STATGRAB/$SEARCH_FOR; then # path given as parameter
     STATGRAB_DIR=$PHP_STATGRAB
  else # search default path list
    AC_MSG_CHECKING([for libstatgrab files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        STATGRAB_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi
  
  if test -z "$STATGRAB_DIR"; then
      AC_MSG_RESULT([not found])
      AC_MSG_ERROR([Please reinstall the libstatgrab distribution from http://www.i-scream.org/libstatgrab/])
  fi

  PHP_ADD_INCLUDE($STATGRAB_DIR/include)
  OLD_CFLAGS=$CFLAGS
  PHP_CHECK_LIBRARY(devstat, devstat_selectdevs, [PHP_ADD_LIBRARY(devstat,,DEVSTAT_SHARED_LIBADD)])
  CFLAGS="$CFLAGS $DEVSTAT_SHARED_LIBADD"

  LIBNAME=statgrab
  LIBSYMBOL=sg_init

  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  [
    PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $STATGRAB_DIR/lib, STATGRAB_SHARED_LIBADD)
    AC_DEFINE(HAVE_STATGRABLIB,1,[ ])
    PHP_STATGRAB_CHECK_VERSION
  ],[
    AC_MSG_ERROR([wrong libstatgrab version or lib not found, Statgrab version 0.10+ is required.])
  ],[
    -L$STATGRAB_DIR/lib -lm
  ])
  dnl
  PHP_SUBST(STATGRAB_SHARED_LIBADD)

  AC_TRY_COMPILE([
#include <statgrab.h>
  ], [
sg_network_iface_stats p;
p.dup = SG_IFACE_DUPLEX_UNKNOWN;
  ], [
    AC_DEFINE(HAVE_OLD_STATGRAB, 1, [ ])
  ])

  PHP_NEW_EXTENSION(statgrab, statgrab.c, $ext_shared)
fi

