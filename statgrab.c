/*
  +----------------------------------------------------------------------+
  | Copyright (c) The PHP Group                                          |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | https://www.php.net/license/3_01.txt                                 |
  +----------------------------------------------------------------------+
  | Author: Ilia Alshanetsky <ilia@ilia.ws>                              |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <statgrab.h>
#include <stdlib.h>
#include <string.h>

#include "php.h"
#include "ext/standard/info.h"
#include "Zend/zend_exceptions.h"
#include "php_statgrab.h"
#include "statgrab_arginfo.h"

static int sg_initialized = 0;

static void php_sg_emit_error(void)
{
	sg_error code = sg_get_error();
	php_error_docref(NULL, E_WARNING, "libstatgrab: %s (code %d)",
		sg_str_error(code), (int)code);
}

#define PHP_SG_ADD_DOUBLE(arr, key, val) \
	add_assoc_double_ex((arr), (key), sizeof(key) - 1, (double)(val))
#define PHP_SG_ADD_LONG(arr, key, val) \
	add_assoc_long_ex((arr), (key), sizeof(key) - 1, (zend_long)(val))
#define PHP_SG_ADD_BOOL(arr, key, val) \
	add_assoc_bool_ex((arr), (key), sizeof(key) - 1, (val) ? 1 : 0)
#define PHP_SG_ADD_STRING_OR_EMPTY(arr, key, val) do {                       \
	const char *_v = (val);                                                  \
	add_assoc_string_ex((arr), (key), sizeof(key) - 1, (char *)(_v ? _v : "")); \
} while (0)

/* ------------------------------------------------------------------- */
/* CPU                                                                  */
/* ------------------------------------------------------------------- */

static void php_sg_cpu_percent(zval *return_value, zend_long source)
{
	size_t entries = 0;
	sg_cpu_percents *cpu = NULL;

	switch (source) {
	case sg_entire_cpu_percent: {
		/* Use the reentrant variant to dodge first-call NULL: sample stats
		 * explicitly and convert via _r. */
		sg_cpu_stats *stats = sg_get_cpu_stats(&entries);
		if (stats == NULL || entries == 0) {
			php_sg_emit_error();
			RETVAL_FALSE;
			return;
		}
		cpu = sg_get_cpu_percents_r(stats, &entries);
		break;
	}
	case sg_last_diff_cpu_percent:
	case sg_new_diff_cpu_percent:
		cpu = sg_get_cpu_percents_of((sg_cpu_percent_source)source, &entries);
		break;
	default:
		php_error_docref(NULL, E_WARNING,
			"invalid CPU percent source " ZEND_LONG_FMT, source);
		RETVAL_FALSE;
		return;
	}

	if (cpu == NULL || entries == 0) {
		php_sg_emit_error();
		RETVAL_FALSE;
		return;
	}

	array_init(return_value);
	PHP_SG_ADD_DOUBLE(return_value, "user",         cpu[0].user);
	PHP_SG_ADD_DOUBLE(return_value, "kernel",       cpu[0].kernel);
	PHP_SG_ADD_DOUBLE(return_value, "idle",         cpu[0].idle);
	PHP_SG_ADD_DOUBLE(return_value, "iowait",       cpu[0].iowait);
	PHP_SG_ADD_DOUBLE(return_value, "swap",         cpu[0].swap);
	PHP_SG_ADD_DOUBLE(return_value, "nice",         cpu[0].nice);
	PHP_SG_ADD_LONG  (return_value, "previous_run", cpu[0].time_taken);
}

static void php_sg_cpu_stats(zval *return_value, int diff)
{
	size_t entries = 0;
	sg_cpu_stats *cpu = diff
		? sg_get_cpu_stats_diff(&entries)
		: sg_get_cpu_stats(&entries);

	if (cpu == NULL || entries == 0) {
		php_sg_emit_error();
		RETVAL_FALSE;
		return;
	}

	array_init(return_value);
	PHP_SG_ADD_LONG(return_value, "user",                          cpu[0].user);
	PHP_SG_ADD_LONG(return_value, "kernel",                        cpu[0].kernel);
	PHP_SG_ADD_LONG(return_value, "idle",                          cpu[0].idle);
	PHP_SG_ADD_LONG(return_value, "iowait",                        cpu[0].iowait);
	PHP_SG_ADD_LONG(return_value, "swap",                          cpu[0].swap);
	PHP_SG_ADD_LONG(return_value, "nice",                          cpu[0].nice);
	PHP_SG_ADD_LONG(return_value, "total",                         cpu[0].total);
	PHP_SG_ADD_LONG(return_value, "context_switches",              cpu[0].context_switches);
	PHP_SG_ADD_LONG(return_value, "voluntary_context_switches",    cpu[0].voluntary_context_switches);
	PHP_SG_ADD_LONG(return_value, "involuntary_context_switches",  cpu[0].involuntary_context_switches);
	PHP_SG_ADD_LONG(return_value, "syscalls",                      cpu[0].syscalls);
	PHP_SG_ADD_LONG(return_value, "interrupts",                    cpu[0].interrupts);
	PHP_SG_ADD_LONG(return_value, "soft_interrupts",               cpu[0].soft_interrupts);
	PHP_SG_ADD_LONG(return_value, "previous_run",                  cpu[0].systime);
}

/* ------------------------------------------------------------------- */
/* Disk I/O                                                             */
/* ------------------------------------------------------------------- */

static void php_sg_diskio(zval *return_value, int diff)
{
	size_t entries = 0;
	sg_disk_io_stats *dst = diff
		? sg_get_disk_io_stats_diff(&entries)
		: sg_get_disk_io_stats(&entries);

	if (dst == NULL) {
		php_sg_emit_error();
		RETVAL_FALSE;
		return;
	}

	array_init(return_value);
	for (size_t i = 0; i < entries; i++) {
		zval row;
		array_init(&row);
		PHP_SG_ADD_LONG(&row, "read",       dst[i].read_bytes);
		PHP_SG_ADD_LONG(&row, "written",    dst[i].write_bytes);
		PHP_SG_ADD_LONG(&row, "time_frame", dst[i].systime);

		if (dst[i].disk_name) {
			add_assoc_zval(return_value, dst[i].disk_name, &row);
		} else {
			add_next_index_zval(return_value, &row);
		}
	}
}

/* ------------------------------------------------------------------- */
/* Filesystems                                                          */
/* ------------------------------------------------------------------- */

static void php_sg_fs(zval *return_value)
{
	size_t entries = 0;
	sg_fs_stats *fs = sg_get_fs_stats(&entries);

	if (fs == NULL) {
		php_sg_emit_error();
		RETVAL_FALSE;
		return;
	}

	array_init(return_value);
	for (size_t i = 0; i < entries; i++) {
		zval row;
		array_init(&row);
		PHP_SG_ADD_STRING_OR_EMPTY(&row, "device_name",      fs[i].device_name);
		PHP_SG_ADD_STRING_OR_EMPTY(&row, "device_canonical", fs[i].device_canonical);
		PHP_SG_ADD_STRING_OR_EMPTY(&row, "fs_type",          fs[i].fs_type);
		PHP_SG_ADD_STRING_OR_EMPTY(&row, "mnt_point",        fs[i].mnt_point);
		PHP_SG_ADD_LONG(&row, "device_type",   fs[i].device_type);
		PHP_SG_ADD_LONG(&row, "size",          fs[i].size);
		PHP_SG_ADD_LONG(&row, "used",          fs[i].used);
		PHP_SG_ADD_LONG(&row, "free",          fs[i].free);
		PHP_SG_ADD_LONG(&row, "avail",         fs[i].avail);
		PHP_SG_ADD_LONG(&row, "total_inodes",  fs[i].total_inodes);
		PHP_SG_ADD_LONG(&row, "used_inodes",   fs[i].used_inodes);
		PHP_SG_ADD_LONG(&row, "free_inodes",   fs[i].free_inodes);
		PHP_SG_ADD_LONG(&row, "avail_inodes",  fs[i].avail_inodes);
		PHP_SG_ADD_LONG(&row, "io_size",       fs[i].io_size);
		PHP_SG_ADD_LONG(&row, "block_size",    fs[i].block_size);
		PHP_SG_ADD_LONG(&row, "total_blocks",  fs[i].total_blocks);
		PHP_SG_ADD_LONG(&row, "free_blocks",   fs[i].free_blocks);
		PHP_SG_ADD_LONG(&row, "used_blocks",   fs[i].used_blocks);
		PHP_SG_ADD_LONG(&row, "avail_blocks",  fs[i].avail_blocks);
		PHP_SG_ADD_LONG(&row, "systime",       fs[i].systime);
		add_next_index_zval(return_value, &row);
	}
}

/* ------------------------------------------------------------------- */
/* Host                                                                 */
/* ------------------------------------------------------------------- */

static void php_sg_host(zval *return_value)
{
	size_t entries = 0;
	sg_host_info *h = sg_get_host_info(&entries);

	if (h == NULL || entries == 0) {
		php_sg_emit_error();
		RETVAL_FALSE;
		return;
	}

	array_init(return_value);
	PHP_SG_ADD_STRING_OR_EMPTY(return_value, "os_name",    h[0].os_name);
	PHP_SG_ADD_STRING_OR_EMPTY(return_value, "os_release", h[0].os_release);
	PHP_SG_ADD_STRING_OR_EMPTY(return_value, "os_version", h[0].os_version);
	PHP_SG_ADD_STRING_OR_EMPTY(return_value, "platform",   h[0].platform);
	PHP_SG_ADD_STRING_OR_EMPTY(return_value, "hostname",   h[0].hostname);
	PHP_SG_ADD_LONG(return_value, "bitwidth",   h[0].bitwidth);
	PHP_SG_ADD_LONG(return_value, "host_state", h[0].host_state);
	PHP_SG_ADD_LONG(return_value, "ncpus",      h[0].ncpus);
	PHP_SG_ADD_LONG(return_value, "maxcpus",    h[0].maxcpus);
	PHP_SG_ADD_LONG(return_value, "uptime",     h[0].uptime);
	PHP_SG_ADD_LONG(return_value, "systime",    h[0].systime);
}

/* ------------------------------------------------------------------- */
/* Load average                                                         */
/* ------------------------------------------------------------------- */

static void php_sg_load(zval *return_value)
{
	size_t entries = 0;
	sg_load_stats *l = sg_get_load_stats(&entries);

	if (l == NULL || entries == 0) {
		php_sg_emit_error();
		RETVAL_FALSE;
		return;
	}

	array_init(return_value);
	PHP_SG_ADD_DOUBLE(return_value, "min1",    l[0].min1);
	PHP_SG_ADD_DOUBLE(return_value, "min5",    l[0].min5);
	PHP_SG_ADD_DOUBLE(return_value, "min15",   l[0].min15);
	PHP_SG_ADD_LONG  (return_value, "systime", l[0].systime);
}

/* ------------------------------------------------------------------- */
/* Memory / swap                                                        */
/* ------------------------------------------------------------------- */

static void php_sg_mem(zval *return_value)
{
	size_t entries = 0;
	sg_mem_stats *m = sg_get_mem_stats(&entries);

	if (m == NULL || entries == 0) {
		php_sg_emit_error();
		RETVAL_FALSE;
		return;
	}

	array_init(return_value);
	PHP_SG_ADD_LONG(return_value, "total",   m[0].total);
	PHP_SG_ADD_LONG(return_value, "free",    m[0].free);
	PHP_SG_ADD_LONG(return_value, "used",    m[0].used);
	PHP_SG_ADD_LONG(return_value, "cache",   m[0].cache);
	PHP_SG_ADD_LONG(return_value, "systime", m[0].systime);
}

static void php_sg_swap(zval *return_value)
{
	size_t entries = 0;
	sg_swap_stats *s = sg_get_swap_stats(&entries);

	if (s == NULL || entries == 0) {
		php_sg_emit_error();
		RETVAL_FALSE;
		return;
	}

	array_init(return_value);
	PHP_SG_ADD_LONG(return_value, "total",   s[0].total);
	PHP_SG_ADD_LONG(return_value, "free",    s[0].free);
	PHP_SG_ADD_LONG(return_value, "used",    s[0].used);
	PHP_SG_ADD_LONG(return_value, "systime", s[0].systime);
}

/* ------------------------------------------------------------------- */
/* Network                                                              */
/* ------------------------------------------------------------------- */

static void php_sg_network(zval *return_value, int diff)
{
	size_t entries = 0;
	sg_network_io_stats *ns = diff
		? sg_get_network_io_stats_diff(&entries)
		: sg_get_network_io_stats(&entries);

	if (ns == NULL) {
		php_sg_emit_error();
		RETVAL_FALSE;
		return;
	}

	array_init(return_value);
	for (size_t i = 0; i < entries; i++) {
		zval row;
		array_init(&row);
		PHP_SG_ADD_LONG(&row, "sent",                ns[i].tx);
		PHP_SG_ADD_LONG(&row, "received",            ns[i].rx);
		PHP_SG_ADD_LONG(&row, "packets_received",    ns[i].ipackets);
		PHP_SG_ADD_LONG(&row, "packets_transmitted", ns[i].opackets);
		PHP_SG_ADD_LONG(&row, "receive_errors",      ns[i].ierrors);
		PHP_SG_ADD_LONG(&row, "transmit_errors",     ns[i].oerrors);
		PHP_SG_ADD_LONG(&row, "collisions",          ns[i].collisions);
		PHP_SG_ADD_LONG(&row, "time_frame",          ns[i].systime);

		if (ns[i].interface_name) {
			add_assoc_zval(return_value, ns[i].interface_name, &row);
		} else {
			add_next_index_zval(return_value, &row);
		}
	}
}

static void php_sg_iface(zval *return_value)
{
	size_t entries = 0;
	sg_network_iface_stats *ifs = sg_get_network_iface_stats(&entries);

	if (ifs == NULL) {
		php_sg_emit_error();
		RETVAL_FALSE;
		return;
	}

	array_init(return_value);
	for (size_t i = 0; i < entries; i++) {
		zval row;
		array_init(&row);
		PHP_SG_ADD_LONG(&row, "speed",   ifs[i].speed);
		PHP_SG_ADD_LONG(&row, "factor",  ifs[i].factor);
		PHP_SG_ADD_LONG(&row, "duplex",  ifs[i].duplex);
		PHP_SG_ADD_BOOL(&row, "active",  ifs[i].up == SG_IFACE_UP);
		PHP_SG_ADD_LONG(&row, "systime", ifs[i].systime);

		if (ifs[i].interface_name) {
			add_assoc_zval(return_value, ifs[i].interface_name, &row);
		} else {
			add_next_index_zval(return_value, &row);
		}
	}
}

/* ------------------------------------------------------------------- */
/* Pages                                                                */
/* ------------------------------------------------------------------- */

static void php_sg_pages(zval *return_value, int diff)
{
	size_t entries = 0;
	sg_page_stats *ps = diff
		? sg_get_page_stats_diff(&entries)
		: sg_get_page_stats(&entries);

	if (ps == NULL || entries == 0) {
		php_sg_emit_error();
		RETVAL_FALSE;
		return;
	}

	array_init(return_value);
	PHP_SG_ADD_LONG(return_value, "pages_in",   ps[0].pages_pagein);
	PHP_SG_ADD_LONG(return_value, "pages_out",  ps[0].pages_pageout);
	PHP_SG_ADD_LONG(return_value, "time_frame", ps[0].systime);
}

/* ------------------------------------------------------------------- */
/* Processes                                                            */
/* ------------------------------------------------------------------- */

static void php_sg_process_count(zval *return_value)
{
	sg_process_count *pc = sg_get_process_count();

	if (pc == NULL) {
		php_sg_emit_error();
		RETVAL_FALSE;
		return;
	}

	array_init(return_value);
	PHP_SG_ADD_LONG(return_value, "total",    pc->total);
	PHP_SG_ADD_LONG(return_value, "running",  pc->running);
	PHP_SG_ADD_LONG(return_value, "sleeping", pc->sleeping);
	PHP_SG_ADD_LONG(return_value, "stopped",  pc->stopped);
	PHP_SG_ADD_LONG(return_value, "zombie",   pc->zombie);
	PHP_SG_ADD_LONG(return_value, "unknown",  pc->unknown);
	PHP_SG_ADD_LONG(return_value, "systime",  pc->systime);
}

static void php_sg_process_stats_impl(zval *return_value, zend_long sort_order, zend_long num_entries, int sort_given)
{
	size_t entries = 0;
	sg_process_stats *ps = sg_get_process_stats(&entries);

	if (ps == NULL) {
		php_sg_emit_error();
		RETVAL_FALSE;
		return;
	}

	if (sort_given) {
		int (*cmp)(const void *, const void *) = NULL;
		switch (sort_order) {
			case 0: cmp = sg_process_compare_name; break;
			case 1: cmp = sg_process_compare_pid;  break;
			case 2: cmp = sg_process_compare_uid;  break;
			case 3: cmp = sg_process_compare_gid;  break;
			case 4: cmp = sg_process_compare_size; break;
			case 5: cmp = sg_process_compare_res;  break;
			case 6: cmp = sg_process_compare_cpu;  break;
			case 7: cmp = sg_process_compare_time; break;
			default:
				php_error_docref(NULL, E_WARNING,
					"'" ZEND_LONG_FMT "' is not a supported sorting mode", sort_order);
				RETVAL_FALSE;
				return;
		}
		qsort(ps, entries, sizeof(*ps), cmp);
	}

	zend_long limit = num_entries;
	if (limit < 1 || (zend_ulong)limit > entries) {
		limit = (zend_long)entries;
	}

	array_init(return_value);
	for (zend_long i = 0; i < limit; i++) {
		zval row;
		array_init(&row);
		PHP_SG_ADD_STRING_OR_EMPTY(&row, "process_name", ps[i].process_name);
		PHP_SG_ADD_STRING_OR_EMPTY(&row, "proc_title",   ps[i].proctitle);
		PHP_SG_ADD_LONG(&row, "pid",         ps[i].pid);
		PHP_SG_ADD_LONG(&row, "parent_pid",  ps[i].parent);
		PHP_SG_ADD_LONG(&row, "leader_pid",  ps[i].pgid);
		PHP_SG_ADD_LONG(&row, "session_id",  ps[i].sessid);
		PHP_SG_ADD_LONG(&row, "uid",         ps[i].uid);
		PHP_SG_ADD_LONG(&row, "gid",         ps[i].gid);
		PHP_SG_ADD_LONG(&row, "euid",        ps[i].euid);
		PHP_SG_ADD_LONG(&row, "egid",        ps[i].egid);
		PHP_SG_ADD_LONG(&row, "size",                         ps[i].proc_size);
		PHP_SG_ADD_LONG(&row, "size_in_mem",                  ps[i].proc_resident);
		PHP_SG_ADD_LONG(&row, "start_time",                   ps[i].start_time);
		PHP_SG_ADD_LONG(&row, "time_spent",                   ps[i].time_spent);
		PHP_SG_ADD_DOUBLE(&row, "cpu_percent",                ps[i].cpu_percent);
		PHP_SG_ADD_LONG(&row, "nice",                         ps[i].nice);
		PHP_SG_ADD_LONG(&row, "state",                        ps[i].state);
		PHP_SG_ADD_LONG(&row, "context_switches",             ps[i].context_switches);
		PHP_SG_ADD_LONG(&row, "voluntary_context_switches",   ps[i].voluntary_context_switches);
		PHP_SG_ADD_LONG(&row, "involuntary_context_switches", ps[i].involuntary_context_switches);
		PHP_SG_ADD_LONG(&row, "systime",                      ps[i].systime);
		add_next_index_zval(return_value, &row);
	}
}

/* ------------------------------------------------------------------- */
/* Users                                                                */
/* ------------------------------------------------------------------- */

static void php_sg_users(zval *return_value)
{
	size_t entries = 0;
	sg_user_stats *us = sg_get_user_stats(&entries);

	if (us == NULL) {
		php_sg_emit_error();
		RETVAL_FALSE;
		return;
	}

	array_init(return_value);
	for (size_t i = 0; i < entries; i++) {
		zval row;
		array_init(&row);
		PHP_SG_ADD_STRING_OR_EMPTY(&row, "login_name", us[i].login_name);
		PHP_SG_ADD_STRING_OR_EMPTY(&row, "device",     us[i].device);
		PHP_SG_ADD_STRING_OR_EMPTY(&row, "hostname",   us[i].hostname);
		PHP_SG_ADD_LONG(&row, "pid",        us[i].pid);
		PHP_SG_ADD_LONG(&row, "login_time", us[i].login_time);
		PHP_SG_ADD_LONG(&row, "systime",    us[i].systime);

		/* record_id may contain non-printable bytes; emit as a length-aware string */
		if (us[i].record_id && us[i].record_id_size > 0) {
			add_assoc_stringl_ex(&row, "record_id", sizeof("record_id") - 1,
				us[i].record_id, us[i].record_id_size);
		} else {
			add_assoc_string_ex(&row, "record_id", sizeof("record_id") - 1, "");
		}

		add_next_index_zval(return_value, &row);
	}
}

/* ------------------------------------------------------------------- */
/* 2.1: valid filesystems / snapshot / error details                    */
/* ------------------------------------------------------------------- */

static void php_sg_valid_filesystems(zval *return_value)
{
	size_t entries = 0;
	const char **fs = sg_get_valid_filesystems(&entries);

	if (fs == NULL) {
		php_sg_emit_error();
		RETVAL_FALSE;
		return;
	}

	array_init(return_value);
	for (size_t i = 0; i < entries; i++) {
		if (fs[i]) {
			add_next_index_string(return_value, fs[i]);
		}
	}
}

static void php_sg_set_valid_filesystems(zval *return_value, zval *zfs)
{
	HashTable *ht = Z_ARRVAL_P(zfs);
	size_t n = zend_hash_num_elements(ht);

	/* libstatgrab wants a NULL-terminated const char *[]. It strdup's the
	 * strings, so freeing our pointer array immediately is safe. */
	const char **buf = (const char **)emalloc(sizeof(const char *) * (n + 1));
	size_t i = 0;

	zval *entry;
	ZEND_HASH_FOREACH_VAL(ht, entry) {
		ZVAL_DEREF(entry);
		if (Z_TYPE_P(entry) != IS_STRING) {
			efree(buf);
			zend_argument_type_error(1,
				"array entries must be strings, %s given at offset %zu",
				zend_zval_value_name(entry), i);
			RETURN_THROWS();
		}
		buf[i++] = Z_STRVAL_P(entry);
	} ZEND_HASH_FOREACH_END();
	buf[i] = NULL;

	sg_error rc = sg_set_valid_filesystems(buf);
	efree(buf);

	if (rc != SG_ERROR_NONE) {
		php_sg_emit_error();
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

static void php_sg_do_snapshot(zval *return_value)
{
	if (sg_snapshot() != SG_ERROR_NONE) {
		php_sg_emit_error();
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

static void php_sg_error_details(zval *return_value)
{
	sg_error_details det;
	memset(&det, 0, sizeof det);

	sg_error code = sg_get_error_details(&det);
	if (code == SG_ERROR_NONE) {
		RETVAL_FALSE;
		return;
	}

	array_init(return_value);
	PHP_SG_ADD_LONG(return_value, "code",  code);
	PHP_SG_ADD_LONG(return_value, "errno", det.errno_value);
	add_assoc_string_ex(return_value, "message", sizeof("message") - 1,
		(char *)sg_str_error(code));
	if (det.error_arg) {
		add_assoc_string_ex(return_value, "arg", sizeof("arg") - 1,
			(char *)det.error_arg);
	} else {
		add_assoc_null_ex(return_value, "arg", sizeof("arg") - 1);
	}
}

/* ------------------------------------------------------------------- */
/* Procedural function dispatchers                                      */
/* ------------------------------------------------------------------- */

#define PHP_SG_PROC_NOARGS(name, body)                          \
	PHP_FUNCTION(name)                                          \
	{                                                           \
		ZEND_PARSE_PARAMETERS_NONE();                           \
		body;                                                   \
	}

PHP_FUNCTION(sg_cpu_percent_usage)
{
	zend_long source = sg_entire_cpu_percent;
	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(source)
	ZEND_PARSE_PARAMETERS_END();
	php_sg_cpu_percent(return_value, source);
}

PHP_SG_PROC_NOARGS(sg_cpu_totals,         php_sg_cpu_stats(return_value, 0))
PHP_SG_PROC_NOARGS(sg_cpu_diff,           php_sg_cpu_stats(return_value, 1))
PHP_SG_PROC_NOARGS(sg_diskio_stats,       php_sg_diskio(return_value, 0))
PHP_SG_PROC_NOARGS(sg_diskio_stats_diff,  php_sg_diskio(return_value, 1))
PHP_SG_PROC_NOARGS(sg_fs_stats,           php_sg_fs(return_value))
PHP_SG_PROC_NOARGS(sg_general_stats,      php_sg_host(return_value))
PHP_SG_PROC_NOARGS(sg_load_stats,         php_sg_load(return_value))
PHP_SG_PROC_NOARGS(sg_memory_stats,       php_sg_mem(return_value))
PHP_SG_PROC_NOARGS(sg_swap_stats,         php_sg_swap(return_value))
PHP_SG_PROC_NOARGS(sg_network_stats,      php_sg_network(return_value, 0))
PHP_SG_PROC_NOARGS(sg_network_stats_diff, php_sg_network(return_value, 1))
PHP_SG_PROC_NOARGS(sg_page_stats,         php_sg_pages(return_value, 0))
PHP_SG_PROC_NOARGS(sg_page_stats_diff,    php_sg_pages(return_value, 1))
PHP_SG_PROC_NOARGS(sg_process_count,      php_sg_process_count(return_value))
PHP_SG_PROC_NOARGS(sg_user_stats,         php_sg_users(return_value))
PHP_SG_PROC_NOARGS(sg_network_iface_stats, php_sg_iface(return_value))
PHP_SG_PROC_NOARGS(sg_valid_filesystems,  php_sg_valid_filesystems(return_value))
PHP_SG_PROC_NOARGS(sg_snapshot,           php_sg_do_snapshot(return_value))
PHP_SG_PROC_NOARGS(sg_error_details,      php_sg_error_details(return_value))

PHP_FUNCTION(sg_set_valid_filesystems)
{
	zval *zfs;
	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ARRAY(zfs)
	ZEND_PARSE_PARAMETERS_END();
	php_sg_set_valid_filesystems(return_value, zfs);
}

PHP_FUNCTION(sg_process_stats)
{
	zend_long sort_order = 0, num_entries = 0;
	zend_bool sort_is_null = 1;

	ZEND_PARSE_PARAMETERS_START(0, 2)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG_OR_NULL(sort_order, sort_is_null)
		Z_PARAM_LONG(num_entries)
	ZEND_PARSE_PARAMETERS_END();

	php_sg_process_stats_impl(return_value, sort_order, num_entries, sort_is_null ? 0 : 1);
}

/* ------------------------------------------------------------------- */
/* OO methods                                                           */
/* ------------------------------------------------------------------- */

PHP_METHOD(Statgrab, __construct)
{
	ZEND_PARSE_PARAMETERS_NONE();
}

#define PHP_SG_METHOD_NOARGS(method, body)                      \
	PHP_METHOD(Statgrab, method)                                \
	{                                                           \
		ZEND_PARSE_PARAMETERS_NONE();                           \
		body;                                                   \
	}

PHP_METHOD(Statgrab, cpu)
{
	zend_long source = sg_entire_cpu_percent;
	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(source)
	ZEND_PARSE_PARAMETERS_END();
	php_sg_cpu_percent(return_value, source);
}

PHP_SG_METHOD_NOARGS(cpuStats,     php_sg_cpu_stats(return_value, 0))
PHP_SG_METHOD_NOARGS(cpuDiff,      php_sg_cpu_stats(return_value, 1))
PHP_SG_METHOD_NOARGS(filesystems,  php_sg_fs(return_value))
PHP_SG_METHOD_NOARGS(host,         php_sg_host(return_value))
PHP_SG_METHOD_NOARGS(load,         php_sg_load(return_value))
PHP_SG_METHOD_NOARGS(memory,       php_sg_mem(return_value))
PHP_SG_METHOD_NOARGS(swap,         php_sg_swap(return_value))
PHP_SG_METHOD_NOARGS(processCount, php_sg_process_count(return_value))
PHP_SG_METHOD_NOARGS(users,             php_sg_users(return_value))
PHP_SG_METHOD_NOARGS(interfaces,        php_sg_iface(return_value))
PHP_SG_METHOD_NOARGS(validFilesystems,  php_sg_valid_filesystems(return_value))
PHP_SG_METHOD_NOARGS(snapshot,          php_sg_do_snapshot(return_value))
PHP_SG_METHOD_NOARGS(errorDetails,      php_sg_error_details(return_value))

PHP_METHOD(Statgrab, setValidFilesystems)
{
	zval *zfs;
	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ARRAY(zfs)
	ZEND_PARSE_PARAMETERS_END();
	php_sg_set_valid_filesystems(return_value, zfs);
}

#define PHP_SG_METHOD_DIFF(method, body)                        \
	PHP_METHOD(Statgrab, method)                                \
	{                                                           \
		zend_bool diff = 0;                                     \
		ZEND_PARSE_PARAMETERS_START(0, 1)                       \
			Z_PARAM_OPTIONAL                                    \
			Z_PARAM_BOOL(diff)                                  \
		ZEND_PARSE_PARAMETERS_END();                            \
		body;                                                   \
	}

PHP_SG_METHOD_DIFF(disks,   php_sg_diskio(return_value,  diff ? 1 : 0))
PHP_SG_METHOD_DIFF(network, php_sg_network(return_value, diff ? 1 : 0))
PHP_SG_METHOD_DIFF(pages,   php_sg_pages(return_value,   diff ? 1 : 0))

PHP_METHOD(Statgrab, processes)
{
	zend_long sort_order = 0, num_entries = 0;
	zend_bool sort_is_null = 1;

	ZEND_PARSE_PARAMETERS_START(0, 2)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG_OR_NULL(sort_order, sort_is_null)
		Z_PARAM_LONG(num_entries)
	ZEND_PARSE_PARAMETERS_END();

	php_sg_process_stats_impl(return_value, sort_order, num_entries, sort_is_null ? 0 : 1);
}

/* ------------------------------------------------------------------- */
/* Module                                                               */
/* ------------------------------------------------------------------- */

#define REGISTER_SG_BC_CONSTANT(c) \
	REGISTER_LONG_CONSTANT(#c, c, CONST_CS | CONST_PERSISTENT)

PHP_MINIT_FUNCTION(statgrab)
{
	if (sg_init(1) != SG_ERROR_NONE) {
		php_error_docref(NULL, E_WARNING,
			"libstatgrab init failed: %s", sg_str_error(sg_get_error()));
		return SUCCESS;
	}
	sg_initialized = 1;

	/* Seed initial snapshots so derived stats (CPU percent, *_diff) are
	 * available on the very first user call. Without this, sg_get_cpu_percents
	 * returns NULL until a second snapshot has been taken. */
	(void)sg_snapshot();

	/* Best-effort privilege drop. Failure is non-fatal: on Linux the library
	 * works fine without setuid bits, and a permission error here used to abort
	 * the whole process via E_ERROR -- bug in the 2006 release. */
	if (sg_drop_privileges() != SG_ERROR_NONE) {
		php_error_docref(NULL, E_NOTICE,
			"libstatgrab: sg_drop_privileges failed: %s",
			sg_str_error(sg_get_error()));
	}

	register_class_Statgrab();

	/* BC: keep the global SG_* longs from the 2006 API. */
	REGISTER_LONG_CONSTANT("SG_FULL_DUPLEX",    SG_IFACE_DUPLEX_FULL,    CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SG_HALF_DUPLEX",    SG_IFACE_DUPLEX_HALF,    CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SG_UNKNOWN_DUPLEX", SG_IFACE_DUPLEX_UNKNOWN, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("SG_PS_SORT_NAME", 0, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SG_PS_SORT_PID",  1, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SG_PS_SORT_UID",  2, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SG_PS_SORT_GID",  3, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SG_PS_SORT_SIZE", 4, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SG_PS_SORT_RES",  5, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SG_PS_SORT_CPU",  6, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SG_PS_SORT_TIME", 7, CONST_CS | CONST_PERSISTENT);

	REGISTER_SG_BC_CONSTANT(SG_PROCESS_STATE_RUNNING);
	REGISTER_SG_BC_CONSTANT(SG_PROCESS_STATE_SLEEPING);
	REGISTER_SG_BC_CONSTANT(SG_PROCESS_STATE_STOPPED);
	REGISTER_SG_BC_CONSTANT(SG_PROCESS_STATE_ZOMBIE);
	REGISTER_SG_BC_CONSTANT(SG_PROCESS_STATE_UNKNOWN);

	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(statgrab)
{
	if (sg_initialized) {
		sg_shutdown();
		sg_initialized = 0;
	}
	return SUCCESS;
}

PHP_MINFO_FUNCTION(statgrab)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "statgrab support",  "enabled");
	php_info_print_table_row(2,    "Extension version", PHP_STATGRAB_VERSION);
	php_info_print_table_row(2,    "libstatgrab",       sg_initialized ? "initialized" : "init failed");
	php_info_print_table_end();
}

zend_module_entry statgrab_module_entry = {
	STANDARD_MODULE_HEADER,
	"statgrab",
	ext_functions,
	PHP_MINIT(statgrab),
	PHP_MSHUTDOWN(statgrab),
	NULL,
	NULL,
	PHP_MINFO(statgrab),
	PHP_STATGRAB_VERSION,
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_STATGRAB
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(statgrab)
#endif
