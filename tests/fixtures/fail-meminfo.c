/* LD_PRELOAD shim: the first fopen("/proc/meminfo") fails with EACCES,
 * so sg_memory_stats() fails once and leaves a libstatgrab error behind. */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <errno.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>

static atomic_int opens;

FILE *fopen(const char *path, const char *mode)
{
	FILE *(*real)(const char *, const char *) =
		(FILE *(*)(const char *, const char *))dlsym(RTLD_NEXT, "fopen");

	if (path != NULL && strcmp(path, "/proc/meminfo") == 0 && atomic_fetch_add(&opens, 1) == 0) {
		errno = EACCES;
		return NULL;
	}
	return real(path, mode);
}
