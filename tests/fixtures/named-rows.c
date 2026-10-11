/* Test-only Linux interposer: predictable rows without privileged devices. */
#include <statgrab.h>
#include <stddef.h>

static char *names[] = {
    "0", "0", NULL, "2", "02", "02", "-1", "-1",
    "11", "9223372036854775807", "sda", "sda", NULL, "", ""
};
#define ROWS (sizeof(names) / sizeof(names[0]))

sg_disk_io_stats *sg_get_disk_io_stats(size_t *entries)
{
    static sg_disk_io_stats rows[ROWS];
    for (size_t i = 0; i < ROWS; i++) {
        rows[i].disk_name = names[i];
        rows[i].read_bytes = i;
    }
    *entries = ROWS;
    return rows;
}

sg_disk_io_stats *sg_get_disk_io_stats_diff(size_t *entries)
{
    return sg_get_disk_io_stats(entries);
}

sg_network_io_stats *sg_get_network_io_stats(size_t *entries)
{
    static sg_network_io_stats rows[ROWS];
    for (size_t i = 0; i < ROWS; i++) {
        rows[i].interface_name = names[i];
        rows[i].tx = i;
    }
    *entries = ROWS;
    return rows;
}

sg_network_io_stats *sg_get_network_io_stats_diff(size_t *entries)
{
    return sg_get_network_io_stats(entries);
}

sg_network_iface_stats *sg_get_network_iface_stats(size_t *entries)
{
    static sg_network_iface_stats rows[ROWS];
    for (size_t i = 0; i < ROWS; i++) {
        rows[i].interface_name = names[i];
        rows[i].speed = i;
    }
    *entries = ROWS;
    return rows;
}
