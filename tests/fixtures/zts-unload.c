/* Embed-SAPI driver: a thread that called statgrab exits only after
 * php_embed_shutdown() has unloaded shared extensions. libstatgrab's
 * pthread key destructor then runs, so the module must still be mapped. */
#include <sapi/embed/php_embed.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef ZTS
# error "zts-unload.c needs a ZTS PHP build"
#endif

ZEND_TSRMLS_CACHE_DEFINE()

static pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
static int phase;

static void set_phase(int p)
{
	pthread_mutex_lock(&mu);
	phase = p;
	pthread_cond_broadcast(&cv);
	pthread_mutex_unlock(&mu);
}

static void wait_phase(int p)
{
	pthread_mutex_lock(&mu);
	while (phase < p) {
		pthread_cond_wait(&cv, &mu);
	}
	pthread_mutex_unlock(&mu);
}

static void *worker(void *arg)
{
	(void)arg;
	(void)ts_resource(0);
	ZEND_TSRMLS_CACHE_UPDATE();
	if (php_request_startup() == SUCCESS) {
		zend_first_try {
			zend_eval_string("echo is_array(sg_fs_stats()) ? \"ok request\\n\" : \"FAIL request\\n\";",
				NULL, "worker");
		} zend_end_try();
		php_request_shutdown(NULL);
	}
	ts_free_thread();
	set_phase(1);
	wait_phase(2);
	return NULL;
}

int main(int argc, char **argv)
{
	if (argc != 3) {
		fprintf(stderr, "usage: %s statgrab.so cycles\n", argv[0]);
		return 2;
	}
	int cycles = atoi(argv[2]);

	for (int i = 0; i < cycles; i++) {
		char ini_path[] = "/tmp/statgrab-unload-ini-XXXXXX";
		int fd = mkstemp(ini_path);
		FILE *f = fd < 0 ? NULL : fdopen(fd, "w");
		if (f == NULL) {
			perror("ini file");
			return 1;
		}
		fprintf(f, "extension=%s\nreport_memleaks=0\n", argv[1]);
		fclose(f);
		setenv("PHPRC", ini_path, 1);

		char *embed_argv[] = { argv[0], NULL };
		int rc = php_embed_init(1, embed_argv);
		unlink(ini_path);
		if (rc == FAILURE) {
			fprintf(stderr, "embed init failed\n");
			return 1;
		}

		pthread_t th;
		set_phase(0);
		if (pthread_create(&th, NULL, worker, NULL) != 0) {
			fprintf(stderr, "pthread_create failed\n");
			return 1;
		}
		wait_phase(1);
		php_embed_shutdown();
		set_phase(2);
		pthread_join(th, NULL);
		printf("ok cycle %d\n", i);
	}
	return 0;
}
