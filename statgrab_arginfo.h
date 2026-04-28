/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 2.0.0-handwritten */

#define ARGINFO_NOARGS_RET_ARRAY_OR_FALSE(name) \
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(name, 0, 0, MAY_BE_ARRAY|MAY_BE_FALSE) \
ZEND_END_ARG_INFO()

ARGINFO_NOARGS_RET_ARRAY_OR_FALSE(arginfo_sg_cpu_percent_usage)
ARGINFO_NOARGS_RET_ARRAY_OR_FALSE(arginfo_sg_cpu_totals)
ARGINFO_NOARGS_RET_ARRAY_OR_FALSE(arginfo_sg_cpu_diff)
ARGINFO_NOARGS_RET_ARRAY_OR_FALSE(arginfo_sg_diskio_stats)
ARGINFO_NOARGS_RET_ARRAY_OR_FALSE(arginfo_sg_diskio_stats_diff)
ARGINFO_NOARGS_RET_ARRAY_OR_FALSE(arginfo_sg_fs_stats)
ARGINFO_NOARGS_RET_ARRAY_OR_FALSE(arginfo_sg_general_stats)
ARGINFO_NOARGS_RET_ARRAY_OR_FALSE(arginfo_sg_load_stats)
ARGINFO_NOARGS_RET_ARRAY_OR_FALSE(arginfo_sg_memory_stats)
ARGINFO_NOARGS_RET_ARRAY_OR_FALSE(arginfo_sg_swap_stats)
ARGINFO_NOARGS_RET_ARRAY_OR_FALSE(arginfo_sg_network_stats)
ARGINFO_NOARGS_RET_ARRAY_OR_FALSE(arginfo_sg_network_stats_diff)
ARGINFO_NOARGS_RET_ARRAY_OR_FALSE(arginfo_sg_page_stats)
ARGINFO_NOARGS_RET_ARRAY_OR_FALSE(arginfo_sg_page_stats_diff)
ARGINFO_NOARGS_RET_ARRAY_OR_FALSE(arginfo_sg_process_count)
ARGINFO_NOARGS_RET_ARRAY_OR_FALSE(arginfo_sg_user_stats)
ARGINFO_NOARGS_RET_ARRAY_OR_FALSE(arginfo_sg_network_iface_stats)

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_sg_process_stats, 0, 0, MAY_BE_ARRAY|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, sort_order, IS_LONG, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, num_entries, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

#define arginfo_class_Statgrab___construct  arginfo_sg_cpu_percent_usage_void
ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Statgrab___construct_, 0, 0, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Statgrab_cpu          arginfo_sg_cpu_percent_usage
#define arginfo_class_Statgrab_cpuStats     arginfo_sg_cpu_totals
#define arginfo_class_Statgrab_cpuDiff      arginfo_sg_cpu_diff
#define arginfo_class_Statgrab_filesystems  arginfo_sg_fs_stats
#define arginfo_class_Statgrab_host         arginfo_sg_general_stats
#define arginfo_class_Statgrab_load         arginfo_sg_load_stats
#define arginfo_class_Statgrab_memory       arginfo_sg_memory_stats
#define arginfo_class_Statgrab_swap         arginfo_sg_swap_stats
#define arginfo_class_Statgrab_processCount arginfo_sg_process_count
#define arginfo_class_Statgrab_users        arginfo_sg_user_stats
#define arginfo_class_Statgrab_interfaces   arginfo_sg_network_iface_stats
#define arginfo_class_Statgrab_processes    arginfo_sg_process_stats

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_Statgrab_disks, 0, 0, MAY_BE_ARRAY|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, diff, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

#define arginfo_class_Statgrab_network arginfo_class_Statgrab_disks
#define arginfo_class_Statgrab_pages   arginfo_class_Statgrab_disks

ZEND_FUNCTION(sg_cpu_percent_usage);
ZEND_FUNCTION(sg_cpu_totals);
ZEND_FUNCTION(sg_cpu_diff);
ZEND_FUNCTION(sg_diskio_stats);
ZEND_FUNCTION(sg_diskio_stats_diff);
ZEND_FUNCTION(sg_fs_stats);
ZEND_FUNCTION(sg_general_stats);
ZEND_FUNCTION(sg_load_stats);
ZEND_FUNCTION(sg_memory_stats);
ZEND_FUNCTION(sg_swap_stats);
ZEND_FUNCTION(sg_network_stats);
ZEND_FUNCTION(sg_network_stats_diff);
ZEND_FUNCTION(sg_page_stats);
ZEND_FUNCTION(sg_page_stats_diff);
ZEND_FUNCTION(sg_process_count);
ZEND_FUNCTION(sg_process_stats);
ZEND_FUNCTION(sg_user_stats);
ZEND_FUNCTION(sg_network_iface_stats);

ZEND_METHOD(Statgrab, __construct);
ZEND_METHOD(Statgrab, cpu);
ZEND_METHOD(Statgrab, cpuStats);
ZEND_METHOD(Statgrab, cpuDiff);
ZEND_METHOD(Statgrab, disks);
ZEND_METHOD(Statgrab, filesystems);
ZEND_METHOD(Statgrab, host);
ZEND_METHOD(Statgrab, load);
ZEND_METHOD(Statgrab, memory);
ZEND_METHOD(Statgrab, swap);
ZEND_METHOD(Statgrab, network);
ZEND_METHOD(Statgrab, pages);
ZEND_METHOD(Statgrab, processes);
ZEND_METHOD(Statgrab, processCount);
ZEND_METHOD(Statgrab, users);
ZEND_METHOD(Statgrab, interfaces);

static const zend_function_entry ext_functions[] = {
	ZEND_FE(sg_cpu_percent_usage,    arginfo_sg_cpu_percent_usage)
	ZEND_FE(sg_cpu_totals,           arginfo_sg_cpu_totals)
	ZEND_FE(sg_cpu_diff,             arginfo_sg_cpu_diff)
	ZEND_FE(sg_diskio_stats,         arginfo_sg_diskio_stats)
	ZEND_FE(sg_diskio_stats_diff,    arginfo_sg_diskio_stats_diff)
	ZEND_FE(sg_fs_stats,             arginfo_sg_fs_stats)
	ZEND_FE(sg_general_stats,        arginfo_sg_general_stats)
	ZEND_FE(sg_load_stats,           arginfo_sg_load_stats)
	ZEND_FE(sg_memory_stats,         arginfo_sg_memory_stats)
	ZEND_FE(sg_swap_stats,           arginfo_sg_swap_stats)
	ZEND_FE(sg_network_stats,        arginfo_sg_network_stats)
	ZEND_FE(sg_network_stats_diff,   arginfo_sg_network_stats_diff)
	ZEND_FE(sg_page_stats,           arginfo_sg_page_stats)
	ZEND_FE(sg_page_stats_diff,      arginfo_sg_page_stats_diff)
	ZEND_FE(sg_process_count,        arginfo_sg_process_count)
	ZEND_FE(sg_process_stats,        arginfo_sg_process_stats)
	ZEND_FE(sg_user_stats,           arginfo_sg_user_stats)
	ZEND_FE(sg_network_iface_stats,  arginfo_sg_network_iface_stats)
	ZEND_FE_END
};

static const zend_function_entry class_Statgrab_methods[] = {
	ZEND_ME(Statgrab, __construct,  arginfo_class_Statgrab___construct_, ZEND_ACC_PUBLIC)
	ZEND_ME(Statgrab, cpu,          arginfo_class_Statgrab_cpu,          ZEND_ACC_PUBLIC)
	ZEND_ME(Statgrab, cpuStats,     arginfo_class_Statgrab_cpuStats,     ZEND_ACC_PUBLIC)
	ZEND_ME(Statgrab, cpuDiff,      arginfo_class_Statgrab_cpuDiff,      ZEND_ACC_PUBLIC)
	ZEND_ME(Statgrab, disks,        arginfo_class_Statgrab_disks,        ZEND_ACC_PUBLIC)
	ZEND_ME(Statgrab, filesystems,  arginfo_class_Statgrab_filesystems,  ZEND_ACC_PUBLIC)
	ZEND_ME(Statgrab, host,         arginfo_class_Statgrab_host,         ZEND_ACC_PUBLIC)
	ZEND_ME(Statgrab, load,         arginfo_class_Statgrab_load,         ZEND_ACC_PUBLIC)
	ZEND_ME(Statgrab, memory,       arginfo_class_Statgrab_memory,       ZEND_ACC_PUBLIC)
	ZEND_ME(Statgrab, swap,         arginfo_class_Statgrab_swap,         ZEND_ACC_PUBLIC)
	ZEND_ME(Statgrab, network,      arginfo_class_Statgrab_network,      ZEND_ACC_PUBLIC)
	ZEND_ME(Statgrab, pages,        arginfo_class_Statgrab_pages,        ZEND_ACC_PUBLIC)
	ZEND_ME(Statgrab, processes,    arginfo_class_Statgrab_processes,    ZEND_ACC_PUBLIC)
	ZEND_ME(Statgrab, processCount, arginfo_class_Statgrab_processCount, ZEND_ACC_PUBLIC)
	ZEND_ME(Statgrab, users,        arginfo_class_Statgrab_users,        ZEND_ACC_PUBLIC)
	ZEND_ME(Statgrab, interfaces,   arginfo_class_Statgrab_interfaces,   ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Statgrab(void)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "Statgrab", class_Statgrab_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL;

	zend_declare_class_constant_long(class_entry, "DUPLEX_FULL",    sizeof("DUPLEX_FULL")-1,    0);
	zend_declare_class_constant_long(class_entry, "DUPLEX_HALF",    sizeof("DUPLEX_HALF")-1,    1);
	zend_declare_class_constant_long(class_entry, "DUPLEX_UNKNOWN", sizeof("DUPLEX_UNKNOWN")-1, 2);

	zend_declare_class_constant_long(class_entry, "SORT_NAME", sizeof("SORT_NAME")-1, 0);
	zend_declare_class_constant_long(class_entry, "SORT_PID",  sizeof("SORT_PID")-1,  1);
	zend_declare_class_constant_long(class_entry, "SORT_UID",  sizeof("SORT_UID")-1,  2);
	zend_declare_class_constant_long(class_entry, "SORT_GID",  sizeof("SORT_GID")-1,  3);
	zend_declare_class_constant_long(class_entry, "SORT_SIZE", sizeof("SORT_SIZE")-1, 4);
	zend_declare_class_constant_long(class_entry, "SORT_RES",  sizeof("SORT_RES")-1,  5);
	zend_declare_class_constant_long(class_entry, "SORT_CPU",  sizeof("SORT_CPU")-1,  6);
	zend_declare_class_constant_long(class_entry, "SORT_TIME", sizeof("SORT_TIME")-1, 7);

	zend_declare_class_constant_long(class_entry, "STATE_RUNNING",  sizeof("STATE_RUNNING")-1,  0);
	zend_declare_class_constant_long(class_entry, "STATE_SLEEPING", sizeof("STATE_SLEEPING")-1, 1);
	zend_declare_class_constant_long(class_entry, "STATE_STOPPED",  sizeof("STATE_STOPPED")-1,  2);
	zend_declare_class_constant_long(class_entry, "STATE_ZOMBIE",   sizeof("STATE_ZOMBIE")-1,   3);
	zend_declare_class_constant_long(class_entry, "STATE_UNKNOWN",  sizeof("STATE_UNKNOWN")-1,  4);

	return class_entry;
}
