/* Embed-SAPI pthread driver: runs PHP scripts concurrently in one ZTS
 * process so races on libstatgrab process globals surface under ASan. */
#include <sapi/embed/php_embed.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef ZTS
# error "zts-driver.c needs a ZTS PHP build"
#endif

ZEND_TSRMLS_CACHE_DEFINE()

#define MAX_THREADS 8

struct job {
	const char *script;
	int requests;
	int seconds;
	int id;
};

static void run_request(const struct job *j, int req)
{
	char code[4352];

	if (php_request_startup() == FAILURE) {
		fprintf(stderr, "thread %d: request startup failed\n", j->id);
		return;
	}
	snprintf(code, sizeof code,
		"$GLOBALS['SG_THREAD'] = %d; $GLOBALS['SG_REQUEST'] = %d; "
		"$GLOBALS['SG_SECONDS'] = %d; require '%s';",
		j->id, req, j->seconds, j->script);
	zend_first_try {
		zend_eval_string(code, NULL, "driver");
	} zend_end_try();
	php_request_shutdown(NULL);
}

static void *worker(void *arg)
{
	struct job *j = arg;

	(void)ts_resource(0);
	ZEND_TSRMLS_CACHE_UPDATE();
	for (int r = 0; r < j->requests; r++) {
		run_request(j, r);
	}
	ts_free_thread();
	return NULL;
}

int main(int argc, char **argv)
{
	if (argc < 5) {
		fprintf(stderr, "usage: %s statgrab.so requests seconds script.php [script.php ...]\n", argv[0]);
		return 2;
	}

	int requests = atoi(argv[2]);
	int seconds = atoi(argv[3]);
	int n = argc - 4;
	if (requests < 1 || seconds < 0 || n > MAX_THREADS) {
		fprintf(stderr, "requests must be >= 1, seconds >= 0, at most %d scripts\n", MAX_THREADS);
		return 2;
	}
	for (int i = 0; i < n; i++) {
		if (strlen(argv[4 + i]) > 4096 || strchr(argv[4 + i], '\'') || strchr(argv[4 + i], '\\')) {
			fprintf(stderr, "unsupported script path: %s\n", argv[4 + i]);
			return 2;
		}
	}

	/* php_embed_init() replaces ini_entries, so load the extension from an ini file. */
	char ini_path[] = "/tmp/statgrab-zts-ini-XXXXXX";
	int fd = mkstemp(ini_path);
	if (fd < 0) {
		perror("mkstemp");
		return 1;
	}
	FILE *f = fdopen(fd, "w");
	if (f == NULL) {
		perror("fdopen");
		close(fd);
		unlink(ini_path);
		return 1;
	}
	fprintf(f, "extension=%s\nmemory_limit=256M\ndisplay_errors=1\nhtml_errors=0\nreport_memleaks=0\n", argv[1]);
	fclose(f);
	setenv("PHPRC", ini_path, 1);

	char *embed_argv[] = { argv[0], NULL };
	if (php_embed_init(1, embed_argv) == FAILURE) {
		fprintf(stderr, "embed init failed\n");
		unlink(ini_path);
		return 1;
	}
	unlink(ini_path);

	pthread_t th[MAX_THREADS];
	struct job jobs[MAX_THREADS];
	int started = 0, rc = 0;
	for (int i = 0; i < n; i++) {
		jobs[i].script = argv[4 + i];
		jobs[i].requests = requests;
		jobs[i].seconds = seconds;
		jobs[i].id = i;
		if (pthread_create(&th[i], NULL, worker, &jobs[i]) != 0) {
			fprintf(stderr, "pthread_create failed for thread %d\n", i);
			rc = 1;
			break;
		}
		started++;
	}
	for (int i = 0; i < started; i++) {
		pthread_join(th[i], NULL);
	}

	php_embed_shutdown();
	return rc;
}
