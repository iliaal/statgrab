/* Test-only Linux interposer: predictable rows without privileged devices. */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <statgrab.h>
#include <stddef.h>

/* Some PHP builds dlopen extensions with RTLD_DEEPBIND, which otherwise
 * bypasses preloaded stat suppliers. Disable it only in this test process. */
#ifdef RTLD_DEEPBIND
void *dlopen(const char *filename, int flags)
{
    void *(*next_dlopen)(const char *, int) = dlsym(RTLD_NEXT, "dlopen");
    return next_dlopen ? next_dlopen(filename, flags & ~RTLD_DEEPBIND) : NULL;
}
#endif

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
