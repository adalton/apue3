#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

struct my_barrier {
	int count;
	pthread_mutex_t mutex;
	pthread_cond_t ready_condition;
};

void
my_barrier_init(struct my_barrier *b, int count)
{
	b->count = count;
	pthread_mutex_init(&b->mutex, NULL);
	pthread_cond_init(&b->ready_condition, NULL);
}

void
my_barrier_wait(struct my_barrier *b)
{
	pthread_mutex_lock(&b->mutex);

	if (--b->count > 0) {
		pthread_cond_wait(&b->ready_condition, &b->mutex);
	}

	pthread_mutex_unlock(&b->mutex);
	pthread_cond_broadcast(&b->ready_condition);
}

void*
my_thread(void *arg)
{
	struct my_barrier *b = arg;

	my_barrier_wait(b);
	fprintf(stderr, "\nThread %lu running", pthread_self());

	pthread_exit(NULL);
}

#define NUM_THREADS 5

int main(void)
{
	struct my_barrier b;

	my_barrier_init(&b, NUM_THREADS);

	pthread_t threads[NUM_THREADS];
	int i;

	for (i = 0; i < NUM_THREADS; ++i) {
		sleep(i);
		fprintf(stderr, "\nStarting thread %d", i);
		pthread_create(&threads[i], NULL, my_thread, &b);
	}

	for (i = 0; i < NUM_THREADS; ++i) {
		pthread_join(threads[i], NULL);
	}

	fprintf(stderr, "\n");
	pthread_exit(NULL);
}
