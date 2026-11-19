/* LD_PRELOAD shim: multiplies the rows libstatgrab reads so a stat call
 * builds a PHP array larger than memory_limit. SG_ROWS (default 20000) sets
 * the row count for the mount table (copies of "/"), /proc/net/dev (copies
 * of "lo"), /proc/diskstats, and the utmp user records. */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <mntent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utmpx.h>

static int rows(void)
{
	const char *n = getenv("SG_ROWS");
	return n ? atoi(n) : 20000;
}

static FILE *fake_file(const char *head, const char *fmt)
{
	int n = rows();
	size_t cap = strlen(head) + (size_t)n * 160 + 1, off;
	char *buf = malloc(cap);

	if (buf == NULL) {
		return NULL;
	}
	off = (size_t)snprintf(buf, cap, "%s", head);
	for (int i = 0; i < n && off < cap; i++) {
		off += (size_t)snprintf(buf + off, cap - off, fmt, i);
	}
	/* Leaked on purpose: fmemopen() reads the buffer until fclose(). */
	return fmemopen(buf, off, "r");
}

static FILE *open_fake(const char *path)
{
	if (strcmp(path, "/proc/net/dev") == 0) {
		return fake_file(
			"Inter-|   Receive                            |  Transmit\n"
			" face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed\n",
			"    lo: %d 1 0 0 0 0 0 0 1 1 0 0 0 0 0 0\n");
	}
	if (strcmp(path, "/proc/diskstats") == 0) {
		return fake_file("", "   8       0 sgdisk%d 1 0 8 0 1 0 8 0 0 0 0\n");
	}
	return NULL;
}

FILE *fopen(const char *path, const char *mode)
{
	FILE *(*real)(const char *, const char *) =
		(FILE *(*)(const char *, const char *))dlsym(RTLD_NEXT, "fopen");
	FILE *f = path ? open_fake(path) : NULL;
	return f ? f : real(path, mode);
}

FILE *fopen64(const char *path, const char *mode)
{
	FILE *(*real)(const char *, const char *) =
		(FILE *(*)(const char *, const char *))dlsym(RTLD_NEXT, "fopen64");
	FILE *f = path ? open_fake(path) : NULL;
	return f ? f : real(path, mode);
}

static __thread int mnt_served;

FILE *setmntent(const char *file, const char *mode)
{
	(void)file;
	(void)mode;
	mnt_served = 0;
	return fmemopen((void *)"", 1, "r");
}

/* Every entry is "/", which statvfs() always resolves, typed ext4 so the
 * default valid-filesystems filter keeps it. */
struct mntent *getmntent_r(FILE *f, struct mntent *mnt, char *buf, int len)
{
	(void)f;
	if (mnt_served >= rows() || len < 32) {
		return NULL;
	}
	mnt_served++;
	memcpy(buf, "sgroot\0ext4\0/\0rw\0", 17);
	mnt->mnt_fsname = buf;
	mnt->mnt_type = buf + 7;
	mnt->mnt_dir = buf + 12;
	mnt->mnt_opts = buf + 14;
	mnt->mnt_freq = 0;
	mnt->mnt_passno = 0;
	return mnt;
}

static __thread int utx_served;
static __thread struct utmpx utx;

void setutxent(void)
{
	utx_served = 0;
}

struct utmpx *getutxent(void)
{
	if (utx_served >= rows()) {
		return NULL;
	}
	memset(&utx, 0, sizeof utx);
	utx.ut_type = USER_PROCESS;
	utx.ut_pid = 1000 + utx_served;
	snprintf(utx.ut_user, sizeof utx.ut_user, "sguser%d", utx_served);
	snprintf(utx.ut_line, sizeof utx.ut_line, "pts/%d", utx_served);
	snprintf(utx.ut_host, sizeof utx.ut_host, "host%d.example", utx_served);
	utx_served++;
	return &utx;
}

void endutxent(void)
{
}
