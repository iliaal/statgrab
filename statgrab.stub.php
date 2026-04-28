<?php

/** @generate-class-entries */

function sg_cpu_percent_usage(): array|false {}
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

    public function __construct() {}

    public function cpu(): array|false {}
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
}
