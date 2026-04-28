/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2004 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.0 of the PHP license,       |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_0.txt.                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Ilia Alshanetsky <ilia@php.net>                              |
  +----------------------------------------------------------------------+
*/

/* $Id: php_statgrab.h,v 1.4 2006/11/07 22:14:38 iliaa Exp $ */

#ifndef PHP_STATGRAB_H
#define PHP_STATGRAB_H

extern zend_module_entry statgrab_module_entry;
#define phpext_statgrab_ptr &statgrab_module_entry

#ifdef PHP_WIN32
#define PHP_STATGRAB_API __declspec(dllexport)
#else
#define PHP_STATGRAB_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(statgrab);
PHP_MINFO_FUNCTION(statgrab);

PHP_FUNCTION(sg_cpu_percent_usage);
PHP_FUNCTION(sg_cpu_totals);
PHP_FUNCTION(sg_cpu_diff);
PHP_FUNCTION(sg_diskio_stats);
PHP_FUNCTION(sg_diskio_stats_diff);
PHP_FUNCTION(sg_fs_stats);
PHP_FUNCTION(sg_general_stats);
PHP_FUNCTION(sg_load_stats);
PHP_FUNCTION(sg_memory_stats);
PHP_FUNCTION(sg_swap_stats);
PHP_FUNCTION(sg_network_stats);
PHP_FUNCTION(sg_network_stats_diff);
PHP_FUNCTION(sg_page_stats);
PHP_FUNCTION(sg_page_stats_diff);
PHP_FUNCTION(sg_process_count);
PHP_FUNCTION(sg_process_stats);
PHP_FUNCTION(sg_user_stats);
PHP_FUNCTION(sg_network_iface_stats);

#endif	/* PHP_STATGRAB_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
