/* Embed host that dlopen()s libphp, runs statgrab, shuts PHP down and
 * dlclose()s it, twice. Neither libphp nor statgrab.so may stay mapped
 * (a system libstatgrab.so may): hosts such as Apache mod_php unload and
 * reload PHP this way. */
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int mapped(const char *needle)
{
	FILE *f = fopen("/proc/self/maps", "r");
	char line[4096];
	int n = 0;

	if (f == NULL) {
		return -1;
	}
	while (fgets(line, sizeof line, f)) {
		if (strstr(line, needle)) {
			n++;
		}
	}
	fclose(f);
	return n;
}

int main(int argc, char **argv)
{
	if (argc != 3) {
		fprintf(stderr, "usage: %s libphp.so statgrab.so\n", argv[0]);
		return 2;
	}
	char ini_path[] = "/tmp/statgrab-dlclose-ini-XXXXXX";
	int fd = mkstemp(ini_path);
	FILE *f = fd < 0 ? NULL : fdopen(fd, "w");
	if (f == NULL) {
		perror("ini file");
		return 1;
	}
	fprintf(f, "extension=%s\nreport_memleaks=0\n", argv[2]);
	fclose(f);
	setenv("PHPRC", ini_path, 1);

	int rc = 0;
	for (int i = 0; i < 2 && rc == 0; i++) {
		void *h = dlopen(argv[1], RTLD_NOW | RTLD_GLOBAL);
		if (h == NULL) {
			fprintf(stderr, "dlopen: %s\n", dlerror());
			rc = 1;
			break;
		}
		int (*init)(int, char **) = (int (*)(int, char **))dlsym(h, "php_embed_init");
		void (*shutdown)(void) = (void (*)(void))dlsym(h, "php_embed_shutdown");
		int (*eval)(const char *, void *, const char *) =
			(int (*)(const char *, void *, const char *))dlsym(h, "zend_eval_string");
		char *embed_argv[] = { argv[0], NULL };
		if (init == NULL || shutdown == NULL || eval == NULL || init(1, embed_argv) != 0) {
			fprintf(stderr, "embed init failed\n");
			rc = 1;
			break;
		}
		eval("echo is_array(sg_fs_stats()) ? \"ok call\\n\" : \"FAIL call\\n\";", NULL, "dlclose");
		fflush(stdout);
		shutdown();
		dlclose(h);
		int php = mapped("libphp"), sg = mapped("/statgrab.so");
		printf("%s cycle %d: libphp maps=%d statgrab.so maps=%d\n",
			php == 0 && sg == 0 ? "ok" : "FAIL", i, php, sg);
		fflush(stdout);
	}
	unlink(ini_path);
	return rc;
}
