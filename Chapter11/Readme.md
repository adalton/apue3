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
 
   Both are legitimate, but the second sequence is slightly better.  With the
   first sequence, it's possible for the kernel to try to schedule a thread that
   was signaled by (iii) before (iv) completes. In that case, the thread calling
   `pthread_cond_broadcast` still holds the mutex, so the thread that was
   signaled would again get blocked when trying to acquire the mutex.  The
   second sequence avoids that releasing the mutex first.

5. What synchronization primitives would you need to implement a barrier?
   Provide an implementation of the `pthread_barrier_wait` function.