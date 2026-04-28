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

/* $Id: statgrab.c,v 1.8 2006/11/07 22:14:38 iliaa Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <statgrab.h>

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h" /* needed for php_explode */
#include "php_statgrab.h"

#define PHP_STATGRAB_VERSION "0.5"

#define PHP_SG_ADD_FVAL(rtz, key, val)	add_assoc_double_ex(rtz, key, sizeof(key), val);
#define PHP_SG_ADD_LVAL(rtz, key, val)	add_assoc_long_ex(rtz, key, sizeof(key), val);
#define PHP_SG_ADD_LLVAL(rtz, key, val)	{	\
	char tmp[256];	\
	int tmp_len = snprintf((char *)&tmp, sizeof(tmp) - 1, "%lld", val);	\
	add_assoc_stringl_ex(rtz, key, sizeof(key), tmp, tmp_len, 1);	\
}

#define PHP_SG_ERROR	\
	{	\
		sg_error sg_errno = sg_get_error();	\
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%d: %s", sg_errno, (char*)sg_str_error(sg_errno)); \
		RETURN_FALSE;	\
	}

/* Process sorting methods */
typedef enum {
	SG_PS_SORT_NAME,
	SG_PS_SORT_PID,
        SG_PS_SORT_UID,
        SG_PS_SORT_GID,
        SG_PS_SORT_SIZE,
	SG_PS_SORT_RES,
        SG_PS_SORT_CPU,
        SG_PS_SORT_TIME
} php_sg_ps_sort;
                                        
#define REGISTER_SG_CONSTANT(__c) REGISTER_LONG_CONSTANT(#__c, __c, CONST_CS | CONST_PERSISTENT)

/* True global resources - no need for thread safety here 
static int le_statgrab;
*/

/* {{{ statgrab_functions[]
 *
 * Every user visible function must have an entry in statgrab_functions[].
 */
function_entry statgrab_functions[] = {
	PHP_FE(sg_cpu_percent_usage,	NULL)
	PHP_FE(sg_cpu_totals,		NULL)
	PHP_FE(sg_cpu_diff,		NULL)
	PHP_FE(sg_diskio_stats,		NULL)
	PHP_FE(sg_diskio_stats_diff,	NULL)
	PHP_FE(sg_fs_stats,		NULL)
	PHP_FE(sg_general_stats,	NULL)
	PHP_FE(sg_load_stats,		NULL)
	PHP_FE(sg_memory_stats,		NULL)
	PHP_FE(sg_swap_stats,		NULL)
	PHP_FE(sg_network_stats,	NULL)
	PHP_FE(sg_network_stats_diff,	NULL)
	PHP_FE(sg_page_stats,		NULL)
	PHP_FE(sg_page_stats_diff,	NULL)
	PHP_FE(sg_process_count,	NULL)
	PHP_FE(sg_process_stats,	NULL)
	PHP_FE(sg_user_stats,		NULL)
	PHP_FE(sg_network_iface_stats,	NULL)
	{NULL, NULL, NULL}	/* Must be the last line in statgrab_functions[] */
};
/* }}} */

/* {{{ statgrab_module_entry
 */
zend_module_entry statgrab_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"statgrab",
	statgrab_functions,
	PHP_MINIT(statgrab),
	NULL,
	NULL,
	NULL,
	PHP_MINFO(statgrab),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_STATGRAB_VERSION, /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_STATGRAB
ZEND_GET_MODULE(statgrab)
#endif

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(statgrab)
{
	/* Initialise statgrab */
	sg_init();
	                
	/* Drop setuid/setgid privileges. */
        if (sg_drop_privileges() != 0) {
		zend_error(E_ERROR, "libstatgrab: Failed to drop privileges");
	}                                                

	REGISTER_LONG_CONSTANT("SG_FULL_DUPLEX",	SG_IFACE_DUPLEX_FULL, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SG_HALF_DUPLEX",	SG_IFACE_DUPLEX_HALF, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SG_UNKNOWN_DUPLEX",	SG_IFACE_DUPLEX_UNKNOWN, CONST_CS|CONST_PERSISTENT);

	/* process sorting order */
	REGISTER_SG_CONSTANT(SG_PS_SORT_NAME);
	REGISTER_SG_CONSTANT(SG_PS_SORT_PID);
	REGISTER_SG_CONSTANT(SG_PS_SORT_UID);
	REGISTER_SG_CONSTANT(SG_PS_SORT_GID);
	REGISTER_SG_CONSTANT(SG_PS_SORT_SIZE);
	REGISTER_SG_CONSTANT(SG_PS_SORT_RES);
	REGISTER_SG_CONSTANT(SG_PS_SORT_CPU);
	REGISTER_SG_CONSTANT(SG_PS_SORT_TIME);

	/* process states */
	REGISTER_SG_CONSTANT(SG_PROCESS_STATE_RUNNING);
	REGISTER_SG_CONSTANT(SG_PROCESS_STATE_SLEEPING);
	REGISTER_SG_CONSTANT(SG_PROCESS_STATE_STOPPED);
	REGISTER_SG_CONSTANT(SG_PROCESS_STATE_ZOMBIE);
	REGISTER_SG_CONSTANT(SG_PROCESS_STATE_UNKNOWN);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(statgrab)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "statgrab support", "enabled");
	php_info_print_table_header(2, "Extension version", PHP_STATGRAB_VERSION " - ($Id: statgrab.c,v 1.8 2006/11/07 22:14:38 iliaa Exp $)");
	php_info_print_table_end();
}
/* }}} */

/* {{{ proto array sg_cpu_percent_usage()
   Return an array indicating the cpu utilization */
PHP_FUNCTION(sg_cpu_percent_usage)
{
	sg_cpu_percents *cpu;

	if (ZEND_NUM_ARGS()) {
		WRONG_PARAM_COUNT;
	}

	cpu = sg_get_cpu_percents();

	array_init(return_value);

	PHP_SG_ADD_FVAL(return_value, "user", cpu->user)
	PHP_SG_ADD_FVAL(return_value, "kernel", cpu->kernel)
	PHP_SG_ADD_FVAL(return_value, "idle", cpu->idle)
	PHP_SG_ADD_FVAL(return_value, "iowait", cpu->iowait)
	PHP_SG_ADD_FVAL(return_value, "swap", cpu->swap)
	PHP_SG_ADD_FVAL(return_value, "nice", cpu->nice)
	PHP_SG_ADD_LVAL(return_value, "previous_run", cpu->time_taken)
}
/* }}} */

static void _php_cpu_stats(INTERNAL_FUNCTION_PARAMETERS, int mode)
{
	sg_cpu_stats *cpu;

	if (ZEND_NUM_ARGS()) {
		WRONG_PARAM_COUNT;
	}

	if (mode) {
		cpu = sg_get_cpu_stats();
	} else {
		cpu = sg_get_cpu_stats_diff();
	}
	if (cpu == NULL) {
		PHP_SG_ERROR;
	}

	array_init(return_value);

	PHP_SG_ADD_LLVAL(return_value, "user", cpu->user)
	PHP_SG_ADD_LLVAL(return_value, "kernel", cpu->kernel)
	PHP_SG_ADD_LLVAL(return_value, "idle", cpu->idle)
	PHP_SG_ADD_LLVAL(return_value, "iowait", cpu->iowait)
	PHP_SG_ADD_LLVAL(return_value, "swap", cpu->swap)
	PHP_SG_ADD_LLVAL(return_value, "nice", cpu->nice)
	PHP_SG_ADD_LLVAL(return_value, "total", cpu->total)
	PHP_SG_ADD_LVAL(return_value, "previous_run", cpu->systime)
}

/* {{{ proto array sg_cpu_totals()
     Returns the total amount of "ticks" the system has spent in each of the different states. */
PHP_FUNCTION(sg_cpu_totals)
{
	_php_cpu_stats(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto array sg_get_cpu_diff()
     Returns the total amount of "ticks" the system has spent in each of the different states since last run. */
PHP_FUNCTION(sg_cpu_diff)
{
	_php_cpu_stats(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

static void _php_sg_diskio(INTERNAL_FUNCTION_PARAMETERS, int mode)
{
	sg_disk_io_stats *dst;
	int ent, i;

	if (ZEND_NUM_ARGS()) {
		WRONG_PARAM_COUNT;
	}

	if (mode) {
		dst = sg_get_disk_io_stats(&ent);
	} else {
		dst = sg_get_disk_io_stats_diff(&ent);
	}
	if (dst == NULL) {
		PHP_SG_ERROR;
	}

	array_init(return_value);

	for (i = 0; i < ent; i++) {
		zval *tmpz;

		MAKE_STD_ZVAL(tmpz);
		array_init(tmpz);

		PHP_SG_ADD_LLVAL(tmpz, "read", dst->read_bytes)
		PHP_SG_ADD_LLVAL(tmpz, "written", dst->write_bytes)
		PHP_SG_ADD_LVAL(tmpz, "time_frame", dst->systime)

		add_assoc_zval(return_value, dst->disk_name, tmpz);

		dst++;
	}
}

/* {{{ proto array sg_diskio_stats()
     Returns the disk IO stored in the kernel which holds the amount of data transferred since bootup. */
PHP_FUNCTION(sg_diskio_stats)
{
	_php_sg_diskio(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto array sg_diskio_stats_diff()
     Returns the disk IO stored in the kernel which holds the amount of data transferred since last call. */
PHP_FUNCTION(sg_diskio_stats_diff)
{
	_php_sg_diskio(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto array sg_fs_stats()
     Returns file system information. */
PHP_FUNCTION(sg_fs_stats)
{
	sg_fs_stats *stats;
	int entries, i;
	zval *this_fs;
	
	if (ZEND_NUM_ARGS()) {
		WRONG_PARAM_COUNT;
	}

	stats = sg_get_fs_stats(&entries);
	if (stats == NULL) {
		PHP_SG_ERROR;
	}

	array_init(return_value);

	for (i = 0; i < entries; i++) {
		MAKE_STD_ZVAL(this_fs);
		array_init(this_fs);

		add_assoc_string(this_fs, "device_name", stats->device_name, 1);
		add_assoc_string(this_fs, "fs_type",  stats->fs_type, 1);
		add_assoc_string(this_fs, "mnt_point",  stats->mnt_point, 1);
		PHP_SG_ADD_LLVAL(this_fs, "size",  stats->size);
		PHP_SG_ADD_LLVAL(this_fs, "used",  stats->used);
		PHP_SG_ADD_LLVAL(this_fs, "avail",  stats->avail);
		PHP_SG_ADD_LLVAL(this_fs, "total_inodes",  stats->total_inodes);
		PHP_SG_ADD_LLVAL(this_fs, "used_inodes",  stats->used_inodes);
		PHP_SG_ADD_LLVAL(this_fs, "free_inodes",  stats->free_inodes);
		PHP_SG_ADD_LLVAL(this_fs, "avail_inodes",  stats->avail_inodes);
		PHP_SG_ADD_LLVAL(this_fs, "io_size",  stats->io_size);
		PHP_SG_ADD_LLVAL(this_fs, "block_size",  stats->block_size);
		PHP_SG_ADD_LLVAL(this_fs, "total_blocks",  stats->total_blocks);
		PHP_SG_ADD_LLVAL(this_fs, "free_blocks",  stats->free_blocks);
		PHP_SG_ADD_LLVAL(this_fs, "used_blocks",  stats->used_blocks);
		PHP_SG_ADD_LLVAL(this_fs, "avail_blocks",  stats->avail_blocks);

		add_next_index_zval(return_value, this_fs);

		*stats++;
	}
}
/* }}} */

/* {{{ proto array sg_general_stats()
     Returns general information about the operating system. */
PHP_FUNCTION(sg_general_stats)
{
	sg_host_info *stats;
	
	if (ZEND_NUM_ARGS()) {
		WRONG_PARAM_COUNT;
	}

	stats = sg_get_host_info();
	if (stats == NULL) {
		PHP_SG_ERROR;
	}

	array_init(return_value);

	add_assoc_string(return_value, "os_name", stats->os_name, 1);
	add_assoc_string(return_value, "os_release", stats->os_release, 1);
	add_assoc_string(return_value, "os_version", stats->os_version, 1);
	add_assoc_string(return_value, "platform", stats->platform, 1);
	add_assoc_string(return_value, "hostname", stats->hostname, 1);
	PHP_SG_ADD_LVAL(return_value, "uptime", stats->uptime)
}
/* }}} */

/* {{{ proto array sg_load_stats()
     Returns the average load for the last 1, 5 and 15 minutes. */
PHP_FUNCTION(sg_load_stats)
{
	sg_load_stats *lst;

	if (ZEND_NUM_ARGS()) {
		WRONG_PARAM_COUNT;
	}

	lst = sg_get_load_stats();
	if (lst == NULL) {
		PHP_SG_ERROR;
	}

	array_init(return_value);

	PHP_SG_ADD_FVAL(return_value, "min1", lst->min1)
	PHP_SG_ADD_FVAL(return_value, "min5", lst->min5)
	PHP_SG_ADD_FVAL(return_value, "min15", lst->min15)
}
/* }}} */

/* {{{ proto array sg_memory_stats()
     Returns information about memory usage. */
PHP_FUNCTION(sg_memory_stats)
{
	sg_mem_stats *ms;

	if (ZEND_NUM_ARGS()) {
		WRONG_PARAM_COUNT;
	}

	ms = sg_get_mem_stats();
	if (ms == NULL) {
		PHP_SG_ERROR;
	}

	array_init(return_value);

	PHP_SG_ADD_LLVAL(return_value, "total", ms->total)
	PHP_SG_ADD_LLVAL(return_value, "free", ms->free)
	PHP_SG_ADD_LLVAL(return_value, "used", ms->used)
	PHP_SG_ADD_LLVAL(return_value, "cache", ms->cache)
}
/* }}} */

/* {{{ proto array sg_swap_stats()
     Returns information about swap usage. */
PHP_FUNCTION(sg_swap_stats)
{
	sg_swap_stats *ss;

	if (ZEND_NUM_ARGS()) {
		WRONG_PARAM_COUNT;
	}

	ss = sg_get_swap_stats();
	if (ss == NULL) {
		PHP_SG_ERROR;
	}

	array_init(return_value);

	PHP_SG_ADD_LLVAL(return_value, "total", ss->total)
	PHP_SG_ADD_LLVAL(return_value, "free", ss->free)
	PHP_SG_ADD_LLVAL(return_value, "used", ss->used)
}
/* }}} */

static void _php_sg_network(INTERNAL_FUNCTION_PARAMETERS, int mode)
{
	sg_network_io_stats *ns;
	int ent, i;

	if (ZEND_NUM_ARGS()) {
		WRONG_PARAM_COUNT;
	}

	if (!mode) {
		ns = sg_get_network_io_stats(&ent);
	} else {
		ns = sg_get_network_io_stats_diff(&ent);
	}
	if (ns == NULL) {
		PHP_SG_ERROR;
	}

	array_init(return_value);
	
	for (i = 0; i < ent; i++) {
		zval *tmpz;

		MAKE_STD_ZVAL(tmpz);
		array_init(tmpz);

		PHP_SG_ADD_LLVAL(tmpz, "sent", ns->tx)
		PHP_SG_ADD_LLVAL(tmpz, "received", ns->rx)
		PHP_SG_ADD_LLVAL(tmpz, "packets_received", ns->ipackets)
		PHP_SG_ADD_LLVAL(tmpz, "packets_transmitted", ns->opackets)
		PHP_SG_ADD_LLVAL(tmpz, "receive_errors", ns->ierrors)
		PHP_SG_ADD_LLVAL(tmpz, "transmit_errors", ns->oerrors)
		PHP_SG_ADD_LLVAL(tmpz, "collisions", ns->collisions)
		PHP_SG_ADD_LVAL(tmpz, "time_frame", ns->systime)

		add_assoc_zval(return_value, ns->interface_name, tmpz);

		ns++;
	}
}

/* {{{ proto array sg_network_stats()
     Returns an array containing transmition statistics for all interfaces. */
PHP_FUNCTION(sg_network_stats)
{
	_php_sg_network(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto array sg_network_stats_diff()
     Returns an array containing transmition statistics for all interfaces since last call. */
PHP_FUNCTION(sg_network_stats_diff)
{
	_php_sg_network(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

static void _php_sg_pages(INTERNAL_FUNCTION_PARAMETERS, int mode)
{
	sg_page_stats *ps;

	if (ZEND_NUM_ARGS()) {
		WRONG_PARAM_COUNT;
	}

	if (mode) {
		ps = sg_get_page_stats();
	} else {
		ps = sg_get_page_stats_diff();
	}
	if (ps == NULL) {
		PHP_SG_ERROR;
	}

	array_init(return_value);

	PHP_SG_ADD_LLVAL(return_value, "pages_in", ps->pages_pagein)
	PHP_SG_ADD_LLVAL(return_value, "pages_out", ps->pages_pageout)
	PHP_SG_ADD_LVAL(return_value, "time_frame", ps->systime)
}

/* {{{ proto array sg_page_stats()
     Return the number of pages the system has paged in and out since bootup. */
PHP_FUNCTION(sg_page_stats)
{
	_php_sg_pages(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto array sg_page_stats_diff()
     Return the number of pages the system has paged in and out since last call to the function. */
PHP_FUNCTION(sg_page_stats_diff)
{
	_php_sg_pages(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto array sg_process_count()
     Returns counts of the currently running processes. */
PHP_FUNCTION(sg_process_count)
{
	sg_process_count *ps;

	if (ZEND_NUM_ARGS()) {
		WRONG_PARAM_COUNT;
	}

	ps = sg_get_process_count();
	if (ps == NULL) {
		PHP_SG_ERROR;
	}

	array_init(return_value);

	PHP_SG_ADD_LVAL(return_value, "total", ps->total)
	PHP_SG_ADD_LVAL(return_value, "running", ps->running)
	PHP_SG_ADD_LVAL(return_value, "sleeping", ps->sleeping)
	PHP_SG_ADD_LVAL(return_value, "stopped", ps->stopped)
	PHP_SG_ADD_LVAL(return_value, "zombie", ps->zombie)
}
/* }}} */

/* {{{ proto array sg_user_stats()
     Returns an array containing all of the currently logged in users. */
PHP_FUNCTION(sg_user_stats)
{
	sg_user_stats *us;
	zval delim, str;

	if (ZEND_NUM_ARGS()) {
		WRONG_PARAM_COUNT;
	}

	us = sg_get_user_stats();
	if (us == NULL) {
		PHP_SG_ERROR;
	}

	array_init(return_value);

	ZVAL_STRINGL(&delim, " ", sizeof(" ") - 1, 0);
	ZVAL_STRINGL(&str, us->name_list, strlen(us->name_list), 0);

	php_explode(&delim, &str, return_value, us->num_entries);
}
/* }}} */

/* {{{ proto array sg_network_iface_stats()
     Returns an array containing statistics about the network interfaces in the machine. */
PHP_FUNCTION(sg_network_iface_stats)
{
	sg_network_iface_stats *ifs;
	int ent, i;

	if (ZEND_NUM_ARGS()) {
		WRONG_PARAM_COUNT;
	}

	ifs = sg_get_network_iface_stats(&ent);

	if (ifs == NULL) {
		PHP_SG_ERROR;
	}

	array_init(return_value);
	
	for (i = 0; i < ent; i++) {
		zval *tmpz;

		MAKE_STD_ZVAL(tmpz);
		array_init(tmpz);

		PHP_SG_ADD_LVAL(tmpz, "speed", ifs->speed)
#ifdef HAVE_OLD_STATGRAB
		PHP_SG_ADD_LVAL(tmpz, "duplex", ifs->dup)
#else
		PHP_SG_ADD_LVAL(tmpz, "duplex", ifs->duplex)
#endif
		add_assoc_bool(tmpz, "active", ifs->up);

		add_assoc_zval(return_value, ifs->interface_name, tmpz);

		ifs++;
	}
}
/* }}} */

/* {{{ proto array sg_process_stats([int sorting_order [, int num_entries]])
     Returns an array containing information about running processes. */
PHP_FUNCTION(sg_process_stats)
{
	sg_process_stats *ps;
	int ent, i;
	long sort_order = -1, num_entries = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &sort_order, &num_entries) == FAILURE) {
		RETURN_FALSE;
	}

	if ((ps = sg_get_process_stats(&ent)) == NULL) {
		PHP_SG_ERROR;
	}

	if (num_entries > ent || num_entries < 1) {
		num_entries = ent;
	}

	if (sort_order != -1) {
		switch (sort_order) {
			case SG_PS_SORT_NAME:
				qsort(ps, ent, sizeof *ps, sg_process_compare_name);
				break;

			case SG_PS_SORT_PID:
				qsort(ps, ent, sizeof *ps, sg_process_compare_pid);
				break;

			case SG_PS_SORT_UID:
				qsort(ps, ent, sizeof *ps, sg_process_compare_uid);
				break;

			case SG_PS_SORT_GID:
				qsort(ps, ent, sizeof *ps, sg_process_compare_gid);
				break;

			case SG_PS_SORT_SIZE:
				qsort(ps, ent, sizeof *ps, sg_process_compare_size);
				break;

			case SG_PS_SORT_RES:
				qsort(ps, ent, sizeof *ps, sg_process_compare_res);
				break;

			case SG_PS_SORT_CPU:
				qsort(ps, ent, sizeof *ps, sg_process_compare_cpu);
				break;

			case SG_PS_SORT_TIME:
				qsort(ps, ent, sizeof *ps, sg_process_compare_time);
				break;
			
			default:
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "'%ld' is not a supported sorting mode.", sort_order);
				RETURN_FALSE;
		}
	}

	array_init(return_value);

	for (i = 0; i < num_entries; i++) {
		zval *tmpz;

		MAKE_STD_ZVAL(tmpz);
		array_init(tmpz);

		add_assoc_string(tmpz, "process_name", ps->process_name ? ps->process_name : "", 1);
		add_assoc_string(tmpz, "proc_title", ps->proctitle ? ps->proctitle: "", 1);
		PHP_SG_ADD_LVAL(tmpz, "pid", ps->pid)
		PHP_SG_ADD_LVAL(tmpz, "parent_pid", ps->parent)
		PHP_SG_ADD_LVAL(tmpz, "leader_pid", ps->pgid)
		PHP_SG_ADD_LVAL(tmpz, "uid", ps->uid)
		PHP_SG_ADD_LVAL(tmpz, "gid", ps->uid)
		PHP_SG_ADD_LVAL(tmpz, "euid", ps->euid)
		PHP_SG_ADD_LVAL(tmpz, "egid", ps->euid)
		PHP_SG_ADD_LVAL(tmpz, "size", ps->proc_size)
		PHP_SG_ADD_LVAL(tmpz, "size_in_mem", ps->proc_resident)
		PHP_SG_ADD_LVAL(tmpz, "time_spent", ps->time_spent)
		PHP_SG_ADD_FVAL(tmpz, "cpu_percent", ps->cpu_percent)
		PHP_SG_ADD_LVAL(tmpz, "nice", ps->nice);
		PHP_SG_ADD_LVAL(tmpz, "state", ps->state);

		add_next_index_zval(return_value, tmpz);

		ps++;
	}
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
