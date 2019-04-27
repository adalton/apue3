#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

struct job {
	struct job *j_next;
	struct job *j_prev;
	pthread_t   j_id;   /* tells which thread handles this job */

	/* ... more stuff here ... */
	int value;
};

struct queue {
	struct job      *q_head;
	struct job      *q_tail;
	pthread_mutex_t  q_lock;
	pthread_cond_t   q_empty_cond;
};

/*
 * Initialize a queue.
 */
int
queue_init(struct queue *qp)
{
	int err;

	qp->q_head = NULL;
	qp->q_tail = NULL;

	err = pthread_mutex_init(&qp->q_lock, NULL);
	if (err != 0)
		return(err);

	err = pthread_cond_init(&qp->q_empty_cond, NULL);
	if (err != 0)
		return(err);

	/* ... continue initialization ... */
	return(0);
}

/*
 * Insert a job at the head of the queue.
 */
void
job_insert(struct queue *qp, struct job *jp)
{
	pthread_mutex_lock(&qp->q_lock);

	jp->j_next = qp->q_head;
	jp->j_prev = NULL;

	if (qp->q_head != NULL)
		qp->q_head->j_prev = jp;
	else
		qp->q_tail = jp;    /* list was empty */

	qp->q_head = jp;

	pthread_mutex_unlock(&qp->q_lock);
	pthread_cond_broadcast(&qp->q_empty_cond);
}

/*
 * Append a job on the tail of the queue.
 */
void
job_append(struct queue *qp, struct job *jp)
{
	pthread_mutex_lock(&qp->q_lock);

	jp->j_next = NULL;
	jp->j_prev = qp->q_tail;

	if (qp->q_tail != NULL)
		qp->q_tail->j_next = jp;
	else
		qp->q_head = jp;    /* list was empty */

	qp->q_tail = jp;

	pthread_mutex_unlock(&qp->q_lock);
	pthread_cond_broadcast(&qp->q_empty_cond);
}

/*
 * Remove the given job from a queue.
 */
void
job_remove(struct queue *qp, struct job *jp)
{
	pthread_mutex_lock(&qp->q_lock);

	if (jp == qp->q_head) {
		qp->q_head = jp->j_next;

		if (qp->q_tail == jp)
			qp->q_tail = NULL;
		else
			jp->j_next->j_prev = jp->j_prev;

	} else if (jp == qp->q_tail) {
		qp->q_tail = jp->j_prev;
		jp->j_prev->j_next = jp->j_next;
	} else {
		jp->j_prev->j_next = jp->j_next;
		jp->j_next->j_prev = jp->j_prev;
	}

	pthread_mutex_unlock(&qp->q_lock);
}

/*
 * Find a job for the given thread ID.
 */
struct job *
job_find(struct queue *qp, pthread_t id)
{
	struct job *jp = NULL;

	if (pthread_mutex_lock(&qp->q_lock) != 0) {
		return(NULL);
	}

	while (jp == NULL) {
		for (jp = qp->q_head; jp != NULL; jp = jp->j_next)
			if (pthread_equal(jp->j_id, id))
				break;

		if (jp == NULL) {
			pthread_cond_wait(&qp->q_empty_cond, &qp->q_lock);
		}
	}

	pthread_mutex_unlock(&qp->q_lock);
	return(jp);
}


void*
worker_thread(void* arg)
{
	struct queue* q = arg;
	const pthread_t id = pthread_self();

	/*
	 * Normally this might loop, handling more than one job, but
	 * for this example, we'll let the thread terminate after handling
	 * a single job.
	 */
	struct job* j = job_find(q, id);
	if (j != NULL) {
		fprintf(stderr, "\nThread %lu doing work %d", id, j->value);
		sleep(j->value);
		job_remove(q, j);
		free(j);
	}

	fprintf(stderr, "\nThread %lu done", id);
	pthread_exit(NULL);
}

#define NUM_THREADS 10

int
main(void)
{
	struct queue q;

	if (queue_init(&q) != 0) {
		fprintf(stderr, "\nFailed to init queue");
	}

	pthread_t threads[NUM_THREADS];
	int i;

	for (i = 0; i < NUM_THREADS; ++i) {
		if (pthread_create(&threads[i], NULL, worker_thread, &q) != 0) {
			fprintf(stderr, "\nFailed to create thread");
			exit(1);
		}
	}

	// Schedule some work
	for (i = 0; i < NUM_THREADS; ++i) {
		struct job* j = malloc(sizeof(struct job));

		j->j_id = threads[i];
		j->value = i;

		/* Just to mix up using job_{insert,append} */
		if (i % 2 == 0) {
			job_insert(&q, j);
		} else {
			job_append(&q, j);
		}
	}

	// Wait for all threads to finish
	for (i = 0; i < NUM_THREADS; ++i) {
		pthread_join(threads[i], NULL);
	}

	fprintf(stderr, "\n");
	pthread_exit(NULL);
}
