/* Linux-only regression driver for the proctitle rewrite in
 * sg_get_process_stats(). libstatgrab reads /proc/PID/cmdline into a buffer
 * of len + 1 bytes, then adjust_procname_cmndline() rewrites it in place.
 * The 0.92.1 code took the first sizeof(char *) bytes of that buffer as a
 * pointer (the OpenBSD kern.proc.args layout); a value equal to buffer + len
 * set len to 0 and sent the rewrite loop past the end of the heap buffer.
 *
 * Making the cmdline bytes name the buffer they are copied into needs the
 * buffer address before the bytes are read. --wrap=open serves this
 * process's /proc/PID/cmdline from a pipe and arms --wrap=sg_realloc, which
 * hands the next allocation of exactly len + 1 bytes the buffer this driver
 * malloc()ed in advance. Only read() runs between the two, so no other
 * allocation can take it. The buffer is ordinary heap memory with its
 * redzone directly after byte len, so under ASan the first out-of-bounds
 * read in the unpatched rewrite loop is always reported. The driver fails
 * if the planted buffer is not the one libstatgrab used, so a change in
 * the allocation path cannot turn the test into a silent pass.
 *
 * A second poll serves an ordinary argv and checks the joined title. */
#define _GNU_SOURCE
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <statgrab.h>

static const char tail_args[] = "\0statgrab-cmdline\0--flag\0";
static const char plain_argv[] = "statgrab-cmdline\0--flag\0";
static const char plain_title[] = "statgrab-cmdline --flag";

static char self_cmdline[64];
static const unsigned char *serve_bytes;
static size_t serve_len;
static int serving;

static void *planted;
static size_t planted_size;
static int armed;
static int consumed;

int __real_open(const char *path, int flags, ...);
void *__real_sg_realloc(void *ptr, size_t size);

void *__wrap_sg_realloc(void *ptr, size_t size)
{
	if (armed && ptr == NULL && size == planted_size) {
		armed = 0;
		consumed = 1;
		return planted;
	}
	return __real_sg_realloc(ptr, size);
}

static int serve_pipe(void)
{
	int fds[2];

	if (pipe2(fds, O_CLOEXEC) != 0) {
		return -1;
	}
	if (write(fds[1], serve_bytes, serve_len) != (ssize_t)serve_len) {
		close(fds[0]);
		close(fds[1]);
		return -1;
	}
	close(fds[1]);
	return fds[0];
}

int __wrap_open(const char *path, int flags, ...)
{
	mode_t mode = 0;

	if ((flags & O_CREAT) || (flags & O_TMPFILE) == O_TMPFILE) {
		va_list ap;

		va_start(ap, flags);
		mode = (mode_t)va_arg(ap, int);
		va_end(ap);
	}
	if (serving && strcmp(path, self_cmdline) == 0) {
		int fd = serve_pipe();

		if (fd != -1 && planted != NULL) {
			armed = 1;
		}
		return fd;
	}
	return __real_open(path, flags, mode);
}

static const sg_process_stats *find_self(const char *tag, size_t *count)
{
	sg_process_stats *ps;
	pid_t self = getpid();

	*count = 0;
	ps = sg_get_process_stats(count);
	if (ps == NULL) {
		printf("FAIL: %s: sg_get_process_stats returned NULL (%s)\n", tag,
		       sg_str_error(sg_get_error()));
		return NULL;
	}
	for (size_t i = 0; i < *count; i++) {
		if (ps[i].pid == self) {
			return &ps[i];
		}
	}
	printf("FAIL: %s: no row for pid %ld\n", tag, (long)self);
	return NULL;
}

/* The title must be the cmdline buffer itself, with some NUL separators
 * turned into spaces, ending inside the buffer. */
static int check_crafted(const sg_process_stats *row, const unsigned char *bytes, size_t len)
{
	size_t tl;

	if (!consumed) {
		printf("FAIL: crafted: libstatgrab did not allocate the planted buffer\n");
		return 1;
	}
	if ((void *)row->proctitle != planted) {
		printf("FAIL: crafted: proctitle %p is not the planted buffer %p\n",
		       (void *)row->proctitle, planted);
		return 1;
	}
	tl = strlen(row->proctitle);
	if (tl > len) {
		printf("FAIL: crafted: title length %zu exceeds cmdline length %zu\n", tl, len);
		return 1;
	}
	for (size_t i = 0; i < tl; i++) {
		unsigned char c = (unsigned char)row->proctitle[i];

		if (c != bytes[i] && !(bytes[i] == '\0' && c == ' ')) {
			printf("FAIL: crafted: title byte %zu is 0x%02x, cmdline byte is 0x%02x\n",
			       i, c, bytes[i]);
			return 1;
		}
	}
	return 0;
}

int main(void)
{
	const sg_process_stats *row;
	unsigned char *crafted;
	size_t crafted_len, count;
	char *end;
	int bad = 0;

	snprintf(self_cmdline, sizeof(self_cmdline), "/proc/%ld/cmdline", (long)getpid());

	if (sg_init(1) != SG_ERROR_NONE) {
		fprintf(stderr, "FAIL: sg_init\n");
		return 1;
	}

	crafted_len = sizeof(char *) + sizeof(tail_args) - 1;
	crafted = malloc(crafted_len);
	planted_size = crafted_len + 1;
	planted = malloc(planted_size);
	if (crafted == NULL || planted == NULL) {
		fprintf(stderr, "FAIL: malloc\n");
		return 1;
	}
	end = (char *)planted + crafted_len;
	memcpy(crafted, &end, sizeof(end));
	memcpy(crafted + sizeof(end), tail_args, sizeof(tail_args) - 1);

	serve_bytes = crafted;
	serve_len = crafted_len;
	serving = 1;
	row = find_self("crafted", &count);
	bad |= row == NULL || check_crafted(row, crafted, crafted_len);
	armed = 0;
	if (!consumed) {
		free(planted);
	}
	planted = NULL;

	serve_bytes = (const unsigned char *)plain_argv;
	serve_len = sizeof(plain_argv) - 1;
	row = find_self("plain", &count);
	if (row == NULL) {
		bad = 1;
	}
	else if (row->proctitle == NULL || strcmp(row->proctitle, plain_title) != 0) {
		printf("FAIL: plain: title \"%s\", want \"%s\"\n",
		       row->proctitle ? row->proctitle : "(null)", plain_title);
		bad = 1;
	}
	serving = 0;

	sg_shutdown();
	free(crafted);

	if (!bad) {
		printf("OK: cmdline naming its own buffer end; plain argv title intact\n");
	}
	return bad;
}
