/* Linux-only regression driver for sg_vector_compute_diff(): serves a
 * crafted /proc/net/dev through --wrap=fopen so the interface count can
 * shrink from above 32 to below it between two diff polls. */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <statgrab.h>

#define TOTAL 40

static const int survivors[] = { 0, 10, 33, 38, 39 };
#define SURVIVORS (sizeof(survivors) / sizeof(survivors[0]))

static char proc_net_dev[65536];
static int serving;

FILE *__real_fopen(const char *path, const char *mode);

FILE *__wrap_fopen(const char *path, const char *mode)
{
	if (serving && strcmp(path, "/proc/net/dev") == 0) {
		return fmemopen(proc_net_dev, strlen(proc_net_dev), "r");
	}
	return __real_fopen(path, mode);
}

static int is_survivor(int idx)
{
	for (size_t k = 0; k < SURVIVORS; k++) {
		if (survivors[k] == idx) {
			return 1;
		}
	}
	return 0;
}

static void serve(int only_survivors, unsigned long long bump)
{
	size_t off = 0;
	int n;

	n = snprintf(proc_net_dev, sizeof(proc_net_dev),
		"Inter-|   Receive                                                |  Transmit\n"
		" face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed\n");
	off = (size_t)n;
	for (int i = 0; i < TOTAL; i++) {
		if (only_survivors && !is_survivor(i)) {
			continue;
		}
		n = snprintf(proc_net_dev + off, sizeof(proc_net_dev) - off,
			"if%d: %llu 10 0 0 0 0 0 0 %llu 10 0 0 0 0 0 0\n",
			i, 1000 + i + bump, 2000 + i + bump);
		if (n < 0 || (size_t)n >= sizeof(proc_net_dev) - off) {
			fprintf(stderr, "FAIL: /proc/net/dev buffer too small\n");
			exit(1);
		}
		off += (size_t)n;
	}
	serving = 1;
}

static int check(const char *tag, const sg_network_io_stats *s, size_t n,
		 size_t want_n, unsigned long long want_delta)
{
	int bad = 0;

	if (s == NULL) {
		printf("FAIL: %s returned NULL (%s)\n", tag, sg_str_error(sg_get_error()));
		return 1;
	}
	if (n != want_n) {
		printf("FAIL: %s entries=%zu, want %zu\n", tag, n, want_n);
		return 1;
	}
	for (size_t i = 0; i < n; i++) {
		if (s[i].rx != want_delta || s[i].tx != want_delta) {
			printf("FAIL: %s %s rx=%llu tx=%llu, want %llu\n", tag,
			       s[i].interface_name ? s[i].interface_name : "(null)",
			       s[i].rx, s[i].tx, want_delta);
			bad = 1;
		}
	}
	return bad;
}

int main(void)
{
	sg_network_io_stats *s;
	size_t n = 0;
	int bad = 0;

	if (sg_init(1) != SG_ERROR_NONE) {
		fprintf(stderr, "FAIL: sg_init\n");
		return 1;
	}

	serve(0, 0);
	s = sg_get_network_io_stats_diff(&n);
	if (s == NULL || n != TOTAL) {
		printf("FAIL: seed diff entries=%zu, want %d\n", n, TOTAL);
		bad = 1;
	}

	serve(0, 100);
	s = sg_get_network_io_stats_diff(&n);
	bad |= check("full diff", s, n, TOTAL, 100);

	serve(1, 107);
	s = sg_get_network_io_stats_diff(&n);
	bad |= check("shrunk diff", s, n, SURVIVORS, 7);

	sg_shutdown();

	if (!bad) {
		printf("OK: %d -> %zu interfaces\n", TOTAL, SURVIVORS);
	}
	return bad;
}
