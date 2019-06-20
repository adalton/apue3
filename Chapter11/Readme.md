1. Modify the example code shown in Figure 11.4 to pass the structure between
   the threads properly.

   The following (`exercise_1.c`) is the modified program.  In this
   implementation, rather than returning a pointer to a stack-allocated
   structure, I dynamically allocate the structure and return that pointer.
   With that, `main` is now responsible for `free`ing that memory.

   An alternative approach would be to allocate the structure in `main`
   and pass its address as the `arg` parameter to `pthread_create`.

   ```c
   #include <stdio.h>
   #include <stdlib.h>
   #include <pthread.h>
   #include <unistd.h>
   
   struct foo {
   	int a, b, c, d;
   };
   
   void
   printfoo(const char *s, const struct foo *fp)
   {
   	printf("%s", s);
   	printf("  structure at 0x%lx\n", (unsigned long)fp);
   	printf("  foo.a = %d\n", fp->a);
   	printf("  foo.b = %d\n", fp->b);
   	printf("  foo.c = %d\n", fp->c);
   	printf("  foo.d = %d\n", fp->d);
   }
   
   void *
   thr_fn1(void *arg)
   {
   	struct foo* foo = malloc(sizeof(struct foo));
   
   	if (foo != NULL) {
   		foo->a = 1;
   		foo->b = 2;
   		foo->c = 3;
   		foo->d = 4;
   
   		printfoo("thread 1:\n", foo);
   	}
   
   	pthread_exit(foo);
   }
   
   void *
   thr_fn2(void *arg)
   {
   	printf("thread 2: ID is %lu\n", (unsigned long)pthread_self());
   
   	pthread_exit(NULL);
   }
   
   int
   main(void)
   {
   	int err;
   	pthread_t tid1;
   	pthread_t tid2;
   	struct foo *fp;
   
   	err = pthread_create(&tid1, NULL, thr_fn1, NULL);
   	if (err != 0) {
   		perror("pthread_create");
   		return err;
   	}
   
   	err = pthread_join(tid1, (void**) &fp);
   	if (err != 0) {
   		perror("pthread_join");
   		return err;
   	}
   
   	sleep(1);
   
   	printf("parent starting second thread\n");
   	err = pthread_create(&tid2, NULL, thr_fn2, NULL);
   	if (err != 0) {
   		perror("pthread_create");
   		return err;
   	}
   
   	sleep(1);
   	printfoo("parent:\n", fp);
   
   	free(fp);
   	exit(0);
   }
   ```

   A sample run of the program looks like:

   ```
   $ ./a.out
   thread 1:
     structure at 0x7f9f5c000b20
     foo.a = 1
     foo.b = 2
     foo.c = 3
     foo.d = 4
   parent starting second thread
   thread 2: ID is 140322535057152
   parent:
     structure at 0x7f9f5c000b20
     foo.a = 1
     foo.b = 2
     foo.c = 3
     foo.d = 4
   ```

2. In the example code shown in Figure 11.14, what additional synchronization
   (if any) is necessary to allow the master thread to change the thread ID
   associated with a pending job? How would this affect the `job_remove`
   function?

   The current synchronization mechanism --- the r/w lock assocaited with
   the queue -- protects the queue.  If we want to support multithreaded
   modification of a job, then we'd need some sort of lock associated with
   each job.  The `job_find` function would then need to lock that lock
   before comparing the thread IDs, and unlock the lock afterwards.  The
   API to change the ID would need to lock the lock, make the modification,
   and unlock the lock. (Here a "lock" could be a r/w lock, mutex, etc.)

   Note that with this configuration, `find_job` could find a job, return it,
   and some other thread could then change the thread ID of that job before
   `job_remove` is called, so clients would need to be able to deal with that.
   An alternative approach would be to have `find_job` remove the job that it
   returns, so that it would be impossible to modify the ID of a job that was
   already in progress.

3. Apply the techniques shown in Figure 11.15 to the worker thread example
   (Figures 11.1 and 11.14) to implement the worker thread function. Don't
   forget to update the `queue_init` function to initialize the condition
   variable and change the `job_insert` and `job_append` functions to signal
   the worker threads. What difficulties arise?

   Here is the implementation; it is also in `exercise_3.c`:

   ```c
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
   ```

   I changed the r/w lock to a mutex, since the condition variable needs a
   mutex.  I added a condition variable to the queue that threads use to wait
   for work when none is available.  I initialize the condition variable in
   `queue_init`.  In `job_find` I block the calling thread in a loop using
   `pthread_cond_wait()` when the queue contains no jobs for the given thread
   ID.  I'd modify `job_insert` and `job_append` to do a
   `pthread_cond_broadcast` to notify any blocked threads that the queue may
   contain work for them.

   With this approach, it's somewhat unfortunate that every thread will
   get unblocked whenever work is added to the queue.  If we have `N`
   threads, all `N` will search the queue for work when we add a job
   for one of them.  We could resolve this by having a queue per thread,
   or by not assigning work to specific threads.

   Note that I didn't add the various `destory` function calls; they seemed
   not contribute to the spirit of this question (and I was lazy).
   
4. Which sequence of steps is correct?
   1. Lock a mutex (`pthread_mutex_lock`).
   2. Change the condition protected by the mutex.
   3. Signal threads waiting on the condition (`pthread_cond_broadcast`).
   4. Unlock the mutex (`pthread_mutex_unlock`).
 
   or
 
   1. Lock a mutex (`pthread_mutex_lock`).
   2. Change the condition protected by the mutex.
   3. Unlock the mutex (`pthread_mutex_unlock`).
   4. Signal threads waiting on the condition (`pthread_cond_broadcast`).
 
   Both are legitimate, but according to the manual the former provides
   "predictable scheduling behavior":

   > The `pthread_cond_broadcast()` or `pthread_cond_signal()` functions may
   > be called by a thread whether or not it currently owns the mutex that
   > threads calling `pthread_cond_wait()` or `pthread_cond_timedwait()` have
   > associated with the condition variable during their waits; however, if
   > predictable scheduling behavior is required, then that mutex shall be
   > locked by the thread calling `pthread_cond_broadcast()` or
   > `pthread_cond_signal()`.

   In either condition, the thread waiting on the condition will need to
   re-evaluate the condition it's waiting for after re-acquiring the lock
   to ensure that no other thread was scheduled that again made the condition
   false.

5. What synchronization primitives would you need to implement a barrier?
   Provide an implementation of the `pthread_barrier_wait` function.

   One approach is to use a mutex and a condition variable.  The following
   is my implmentation of `pthread_barrier_wait` along with a test application
   (also in `exercise_5.c`).  Note the call to `sleep` so that all of the
   threads do not get created within a short amount of time. As soon at the
   last threads starts, all of the "running" messages are printed.

   ```c
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
   my_barrier_destroy(struct my_barrier *b)
   {
   	pthread_cond_destroy(&b->ready_condition);
   	pthread_mutex_destroy(&b->mutex);
   }
   
   int
   my_barrier_wait(struct my_barrier *b)
   {
   	int rc = PTHREAD_BARRIER_SERIAL_THREAD;
   	pthread_mutex_lock(&b->mutex);
   
   	if (--b->count > 0) {
   		pthread_cond_wait(&b->ready_condition, &b->mutex);
   		rc = 0;
   	}
   
   	pthread_mutex_unlock(&b->mutex);
   	pthread_cond_broadcast(&b->ready_condition);
   
   	return rc;
   }
   
   void*
   my_thread(void *arg)
   {
   	struct my_barrier *b = arg;
   	int ret = my_barrier_wait(b);
   
   	fprintf(stderr, "\nret = %d; thread %lu running", ret, pthread_self());
   
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
   
   	my_barrier_destroy(&b);
   
   	fprintf(stderr, "\n");
   	pthread_exit(NULL);
   }
   ```

   Here's a sample run:

   ```
   $ ./a.out
   
   Starting thread 0
   Starting thread 1
   Starting thread 2
   Starting thread 3
   Starting thread 4
   ret = 0; thread 139688797914880 running
   ret = 0; thread 139688789522176 running
   ret = 0; thread 139688781129472 running
   ret = 0; thread 139688772736768 running
   ret = -1; thread 139688764344064 running
   ```
