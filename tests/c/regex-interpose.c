/* Linux regression driver: the program exports its own regcomp(),
 * regexec(), and regfree(), as PHP does when it links oniguruma's libonig.
 * libstatgrab must still parse /proc/net/dev and /proc/filesystems with
 * glibc's regex. */
#define _GNU_SOURCE
#include <regex.h>
#include <stdio.h>
#include <statgrab.h>

static int foreign_calls;

int regcomp(regex_t *preg, const char *regex, int cflags)
{
	(void)preg;
	(void)regex;
	(void)cflags;
	foreign_calls++;
	return REG_ESPACE;
}

int regexec(const regex_t *preg, const char *string, size_t nmatch, regmatch_t pmatch[], int eflags)
{
	(void)preg;
	(void)string;
	(void)nmatch;
	(void)pmatch;
	(void)eflags;
	foreign_calls++;
	return REG_NOMATCH;
}

void regfree(regex_t *preg)
{
	(void)preg;
	foreign_calls++;
}

int main(void)
{
	size_t ifaces = 0, fs_types = 0;
	int rc = 0;

	sg_init(1);
	sg_get_network_io_stats(&ifaces);
	sg_get_valid_filesystems(&fs_types);
	printf("network rows=%zu valid filesystems=%zu foreign regex calls=%d\n",
		ifaces, fs_types, foreign_calls);
	if (ifaces == 0 || fs_types == 0 || foreign_calls != 0) {
		printf("FAIL: libstatgrab used the program's regex functions\n");
		rc = 1;
	} else {
		printf("PASS\n");
	}
	sg_shutdown();
	return rc;
}
