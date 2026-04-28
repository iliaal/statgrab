/*
  +----------------------------------------------------------------------+
  | Copyright (c) The PHP Group                                          |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,     |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | https://www.php.net/license/3_01.txt                                 |
  +----------------------------------------------------------------------+
  | Author: Ilia Alshanetsky <ilia@ilia.ws>                              |
  +----------------------------------------------------------------------+
*/

#ifndef PHP_STATGRAB_H
#define PHP_STATGRAB_H

#define PHP_STATGRAB_VERSION "2.1.0"

extern zend_module_entry statgrab_module_entry;
#define phpext_statgrab_ptr &statgrab_module_entry

#ifdef PHP_WIN32
# define PHP_STATGRAB_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
# define PHP_STATGRAB_API __attribute__((visibility("default")))
#else
# define PHP_STATGRAB_API
#endif

#ifdef ZTS
# include "TSRM.h"
#endif

#endif	/* PHP_STATGRAB_H */
