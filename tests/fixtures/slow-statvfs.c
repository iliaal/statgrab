/* LD_PRELOAD shim: statvfs()/statvfs64() on $SG_SLOW_PATH sleeps
 * $SG_SLOW_MS ms first, standing in for a slow or hung network mount. */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>
#include <unistd.h>

static atomic_int hits;

static void slow_down(const char *path)
{
	const char *slow = getenv("SG_SLOW_PATH");
	const char *ms = getenv("SG_SLOW_MS");

	if (slow != NULL && path != NULL && strcmp(slow, path) == 0) {
		atomic_fetch_add(&hits, 1);
		usleep((useconds_t)(ms ? atoi(ms) : 2000) * 1000);
	}
}

int statvfs64(const char *path, struct statvfs64 *buf)
{
	int (*real)(const char *, struct statvfs64 *) =
		(int (*)(const char *, struct statvfs64 *))dlsym(RTLD_NEXT, "statvfs64");
	slow_down(path);
	return real(path, buf);
}

int statvfs(const char *path, struct statvfs *buf)
{
	int (*real)(const char *, struct statvfs *) =
		(int (*)(const char *, struct statvfs *))dlsym(RTLD_NEXT, "statvfs");
	slow_down(path);
	return real(path, buf);
}

__attribute__((destructor)) static void report(void)
{
	fprintf(stderr, "slow statvfs hits: %d\n", atomic_load(&hits));
}
