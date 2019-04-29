/*
6. Reimplement the program in Figure 10.29 to make it thread-safe without using
   `nanosleep` or `clock_nanosleep`.
*/
#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

unsigned
sleep(const unsigned seconds)
{
	struct timeval timeout = { .tv_sec = seconds };

	(void) select(0, NULL, NULL, NULL, &timeout);

	return timeout.tv_sec;
}

static void
sig_handler(const int signo)
{
	// Just return. This should interrupt the all to select
}

static void*
thread_main(void* args)
{
	const int* const thread_num = args;

	// Add 20 just to have more time to play
	const int time_remaining = sleep(20 + *thread_num);

	printf("Thread %d woke up %d seconds early\n",
	       *thread_num,
	       time_remaining);

	pthread_exit(NULL);
}

#define NUM_THREADS 5

int
main(void)
{
	if (signal(SIGHUP, sig_handler) < 0) {
		perror("signal");
		return 1;
	}

	int i;
	int thread_args[NUM_THREADS];
	pthread_t threads[NUM_THREADS];

	for (i = 0; i < NUM_THREADS; ++i) {
		thread_args[i] = i;
		assert(pthread_create(&threads[i], NULL, thread_main, &thread_args[i]) == 0);
	}

	for (i = 0; i < NUM_THREADS; ++i) {
		pthread_join(threads[i], NULL);
	}

	return 0;
}
