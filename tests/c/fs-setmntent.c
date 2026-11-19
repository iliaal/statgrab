/* Linux regression driver for sg_get_fs_stats(): every setmntent() fails
 * through --wrap=setmntent, and the failure must stay readable through
 * sg_get_error() after the call returns NULL. */
#define _GNU_SOURCE
#include <errno.h>
#include <mntent.h>
#include <stdio.h>
#include <statgrab.h>

FILE *__wrap_setmntent(const char *file, const char *mode)
{
	(void)file;
	(void)mode;
	errno = EACCES;
	return NULL;
}

int main(void)
{
	size_t entries = 99;
	sg_error_details det = { 0 };
	int rc = 0;

	if (sg_init(1) != SG_ERROR_NONE) {
		fprintf(stderr, "sg_init failed\n");
		return 1;
	}
	sg_fs_stats *fs = sg_get_fs_stats(&entries);
	sg_error err = sg_get_error();
	sg_get_error_details(&det);
	printf("fs=%s entries=%zu error=%s (%d) errno=%d\n", fs ? "non-NULL" : "NULL",
		entries, sg_str_error(err), (int)err, det.errno_value);
	if (fs != NULL || entries != 0 || err != SG_ERROR_SETMNTENT || det.errno_value != EACCES) {
		printf("FAIL: expected NULL, 0 entries, SG_ERROR_SETMNTENT and EACCES\n");
		rc = 1;
	} else {
		printf("PASS\n");
	}
	sg_shutdown();
	return rc;
}
