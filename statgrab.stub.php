<?php

/** @generate-class-entries */

function sg_cpu_percent_usage(int $source = 0): array|false {}
function sg_cpu_totals(): array|false {}
function sg_cpu_diff(): array|false {}
function sg_diskio_stats(): array|false {}
function sg_diskio_stats_diff(): array|false {}
function sg_fs_stats(): array|false {}
function sg_general_stats(): array|false {}
function sg_load_stats(): array|false {}
function sg_memory_stats(): array|false {}
function sg_swap_stats(): array|false {}
function sg_network_stats(): array|false {}
function sg_network_stats_diff(): array|false {}
function sg_page_stats(): array|false {}
function sg_page_stats_diff(): array|false {}
function sg_process_count(): array|false {}
function sg_process_stats(?int $sort_order = null, int $num_entries = 0): array|false {}
function sg_user_stats(): array|false {}
function sg_network_iface_stats(): array|false {}

/* 2.1.0 additions */
function sg_valid_filesystems(): array|false {}
function sg_set_valid_filesystems(array $filesystems): bool {}
function sg_snapshot(): bool {}
function sg_error_details(): array|false {}

final class Statgrab
{
    /* duplex */
    const DUPLEX_FULL    = 0;
    const DUPLEX_HALF    = 1;
    const DUPLEX_UNKNOWN = 2;

    /* process sort orders */
    const SORT_NAME = 0;
    const SORT_PID  = 1;
    const SORT_UID  = 2;
    const SORT_GID  = 3;
    const SORT_SIZE = 4;
    const SORT_RES  = 5;
    const SORT_CPU  = 6;
    const SORT_TIME = 7;

    /* process states */
    const STATE_RUNNING  = 0;
    const STATE_SLEEPING = 1;
    const STATE_STOPPED  = 2;
    const STATE_ZOMBIE   = 3;
    const STATE_UNKNOWN  = 4;

    /* CPU percent sources (2.1) */
    const CPU_PERCENT_ENTIRE    = 0;
    const CPU_PERCENT_LAST_DIFF = 1;
    const CPU_PERCENT_NEW_DIFF  = 2;

    /* host_state values (2.1) */
    const HOST_STATE_UNKNOWN              = 0;
    const HOST_STATE_PHYSICAL             = 1;
    const HOST_STATE_VIRTUAL_MACHINE      = 2;
    const HOST_STATE_PARAVIRTUAL_MACHINE  = 3;
    const HOST_STATE_HARDWARE_VIRTUALIZED = 4;

    /* fs device_type bitmask (2.1) */
    const FS_UNKNOWN  = 0;
    const FS_REGULAR  = 1;
    const FS_SPECIAL  = 2;
    const FS_LOOPBACK = 4;
    const FS_REMOTE   = 8;
    const FS_LOCAL    = 3;     /* REGULAR | SPECIAL */
    const FS_ALLTYPES = 15;    /* REGULAR | SPECIAL | LOOPBACK | REMOTE */

    /* sg_error codes (2.1) — handful most useful for callers */
    const ERROR_NONE             = 0;
    const ERROR_INVALID_ARGUMENT = 1;
    const ERROR_OPEN             = 21;
    const ERROR_OPENDIR          = 22;
    const ERROR_PERMISSION       = 29;
    const ERROR_UNSUPPORTED      = 41;

    public function __construct() {}

    public function cpu(int $source = self::CPU_PERCENT_ENTIRE): array|false {}
    public function cpuStats(): array|false {}
    public function cpuDiff(): array|false {}
    public function disks(bool $diff = false): array|false {}
    public function filesystems(): array|false {}
    public function host(): array|false {}
    public function load(): array|false {}
    public function memory(): array|false {}
    public function swap(): array|false {}
    public function network(bool $diff = false): array|false {}
    public function pages(bool $diff = false): array|false {}
    public function processes(?int $sort_order = null, int $num_entries = 0): array|false {}
    public function processCount(): array|false {}
    public function users(): array|false {}
    public function interfaces(): array|false {}

    /* 2.1 additions */
    public function validFilesystems(): array|false {}
    public function setValidFilesystems(array $filesystems): bool {}
    public function snapshot(): bool {}
    public function errorDetails(): array|false {}
}
