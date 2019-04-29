1. Run the program in Figure 12.17 on a Linux system, but redirect the output
   into a file. Explain the results.

   Running normally:

   ```
   $ ./a.out
   thread started...
   parent about to fork...
   preparing locks...
   parent unlocking locks...
   parent returned from fork
   child unlocking locks...
   child returned from fork
   ```

   Running with output redirected to a file:

   ```
   $ ./a.out > /tmp/out && cat /tmp/out
   thread started...
   parent about to fork...
   preparing locks...
   parent unlocking locks...
   parent returned from fork
   thread started...
   parent about to fork...
   preparing locks...
   child unlocking locks...
   child returned from fork
   ```

   _Explain the results._

   When writing to the terminal device, the output was line-buffered; the
   buffer was flushed when a newline was encountered.  As a result, every
   call to `printf` was immediately flushed to standard output.

   When standard output is redirected to the file, the output was
   fully-buffered.  At the point where the process `fork`ed, the following
   output was in the buffer; that buffer was copied to the child during the
   `fork`:

   ```
   thread started...
   parent about to fork...
   preparing locks...
   ```

   As the parent and child continued execution, each continued to append to
   their respective buffers.  When the parent `exit`ed, it flushed its buffer,
   writing:

   ```
   thread started...
   parent about to fork...
   preparing locks...
   parent unlocking locks...
   parent returned from fork
   ```
   
   Finally, when the child `exit`ed, it flushed its buffer, writing:

   ```
   thread started...
   parent about to fork...
   preparing locks...
   child unlocking locks...
   child returned from fork
   ```

2. Implement `putenv_r`, a reentrant version of `putenv`. Make sure that your
   implementation is async-signal safe as well as thread-safe.

   Here's the implementation (also in `exercise_2.c`):

   ```c
   #include <pthread.h>
   #include <signal.h>
   #include <stdio.h>
   #include <stdlib.h>
   
   int
   my_putenvr(char* string)
   {
   	static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
   
   	int retval = -1;
   	int rc;
   	sigset_t set = {};
   	sigset_t oldset = {};
   
   	rc = sigfillset(&set);
   	if (rc < 0) {
   		return rc;
   	}
   
   	/* Mask all signals, save existing mask */
   	rc = sigprocmask(SIG_SETMASK, &set, &oldset);
   	if (rc < 0) {
   		return rc;
   	}
   
   	if (pthread_mutex_lock(&mutex) != 0) {
   		retval = -1;
   		goto restore_signals;
   	}
   
   	retval = putenv(string);
   
   	if (pthread_mutex_unlock(&mutex) != 0) {
   		/*
   		 * We locked the lock successfully but failed to unlock it?
   		 * It might be better to terminate here
   		 */
   		if (retval == 0) {
   			retval = -1;
   		}
   	}
   
   restore_signals:
   	/* Restore previously-masked signals */
   	rc = sigprocmask(SIG_SETMASK, &oldset, NULL);
   	if (retval == 0) {
   		retval = rc;
   	}
   
   	return retval;
   }
   
   int
   main(void)
   {
   	my_putenvr("x=y");
   	printf("x=%s\n", getenv("x"));
   
   	my_putenvr("x=z");
   	printf("x=%s\n", getenv("x"));
   
   	return 0;
   }
   ```

3. Can you make the `getenv` function shown in Figure 12.13 async-signal safe
   by blocking signals at the beginning of the function and restoring the
   previous signal mask before returning? Explain.

   No. Blocking signals for the duration of `getenv` will protect `getenv`, but
   the function returns a pointer to thread-local memory.  After `getenv`
   returns, if a signal fires against the thread, and if that signal's handler
   also calls `getenv`, then the signal handler's call will overwrite the
   value in the thread-local memory buffer.  If the signal handler returns,
   when control returns to the original function, the value it requested via
   `getenv` will have been overwritten.

4. Write a program to exercise the version of `getenv` from Figure 12.13.
   Compile and run the program on FreeBSD. What happens? Explain.

   Here's the program (also in `exercise_4.c`):

   ```c
   #include <limits.h>
   #include <stdio.h>
   #include <stdlib.h>
   #include <string.h>
   #include <pthread.h>
   
   #define MAXSTRINGSZ 4096
   
   static pthread_key_t key;
   static pthread_once_t init_done = PTHREAD_ONCE_INIT;
   pthread_mutex_t env_mutex = PTHREAD_MUTEX_INITIALIZER;
   
   extern char **environ;
   
   static void
   thread_init(void)
   {
   	pthread_key_create(&key, free);
   }
   
   char *
   getenv(const char *name)
   {
   	int     i, len;
   	char    *envbuf;
   
   	pthread_once(&init_done, thread_init);
   	pthread_mutex_lock(&env_mutex);
   
   	envbuf = (char *)pthread_getspecific(key);
   	if (envbuf == NULL) {
   		envbuf = malloc(MAXSTRINGSZ);
   		if (envbuf == NULL) {
   			pthread_mutex_unlock(&env_mutex);
   			return(NULL);
   		}
   		pthread_setspecific(key, envbuf);
   	}
   
   	len = strlen(name);
   	for (i = 0; environ[i] != NULL; i++) {
   		if ((strncmp(name, environ[i], len) == 0) &&
   				(environ[i][len] == '=')) {
   			strncpy(envbuf, &environ[i][len + 1], MAXSTRINGSZ -1);
   			pthread_mutex_unlock(&env_mutex);
   			return(envbuf);
   		}
   	}
   
   	pthread_mutex_unlock(&env_mutex);
   
   	return(NULL);
   }
   
   int main(int argc, char* argv[])
   {
   	if (argc < 2) {
   		fprintf(stderr, "Usage: %s <varname>\n", argv[0]);
   		return 1;
   	}
   
   	const char* const var = getenv(argv[1]);
   	printf("%s = %s\n", argv[1], var ? var : "");
   
   	return 0;
   }
   ```

   On Linux it works as expected.  On FreeBSD the program runs for a few
   seconds, then crashes.  Running `gdb` on the program I see the following
   function call sequence:

   ```
   ... 
   ... in pthread_timedjoin_np () from /lib/libthr.so.3
   ... in pthread_once () from /lib/libthr.so.3
   ... in getenv ()
   ... in pthread_timedjoin_np () from /lib/libthr.so.3
   ... in pthread_once () from /lib/libthr.so.3
   ... in getenv ()
   ... in pthread_timedjoin_np () from /lib/libthr.so.3
   ... in pthread_once () from /lib/libthr.so.3
   ... in getenv ()
   ... nallocm () from /lib/libc.so.7
   ... _pthread_cancel_leave () from /lib/libc.so.7
   ... .init () from /lib/libc.so.7
   ```

   Here, we haven't even started executing `main` yet; we're still in
   pre-`main` initialization code.  `.init` calls `_pthread_cancel_leave`,
   which calls `nallocm`.  `nallocm` calls our implementation of `getenv`
   looking for the environment variable `MALLOC_CONF`.  Since this is the
   first call to our `getenv`, it calls `pthread_once`, which calls
   `pthread_timedjoin_np`.  `pthread_timedjoin_np` then calls `getenv`
   (recursively) looking for the environment variable `LIBPTHREAD_SPINLOOPS`.
   Since the first call to `pthread_once` hasn't yet completed, the fact that
   it has been called hasn't yet been recorded, so our `getenv` calls
   `pthread_once` again.  The recursive call cycle continues until the program
   overflows its call stack, at which time it receives a `SIGSEGV` and
   terminates.

5. Given that you can create multiple threads to perform different tasks within
   a program, explain why you might still need to use `fork`.

   Threads run in the same address space, so an error in the code running
   in one thread can affect another.  With `fork`, the child is a separate
   process that can't (directly) affect the state of the parent.

   Additionally, some system calls cannot be used by multithreaded programs.
   For example, the `setns` system call in Linux --- which is used to switch
   user namespaces --- cannot be called by multithreaded processes to switch
   to some some namespaces. If a multithreaded process needs to perform some
   action in a different namespace, it may need to fork first.

6. Reimplement the program in Figure 10.29 to make it thread-safe without using
   `nanosleep` or `clock_nanosleep`.

   This implementation will work for Linux, but probably not other flavors
   of Unix:

   ```c
   unsigned int
   sleep(const unsigned int seconds)
   {
   	struct timeval timeout = { .tv_sec = seconds };

   	(void) select(0, NULL, NULL, NULL, &timeout);

   	return timeout.tv_sec;
   }
   ```

   From the `select` man page:

   > On  Linux, `select()` modifies `timeout` to reflect the amount of time
   > not slept; most other implementations do not do this.   (POSIX.1  permits
   > either  behavior.)   This causes problems both when Linux code which
   > reads timeout is ported to other operating systems, and when code is
   > ported to Linux that reuses a `struct timeval` for  multiple `select()`s
   > in a loop without reinitializing it.

   That said, for the sake of this question, I'll stick with it :).

7. After calling `fork`, could we safely reinitialize a condition variable in
   the child process by first destroying the condition variable with
   `pthread_cond_destroy` and then initializing it with `pthread_cond_init`?

   No.  According to the manual for `pthread_cond_destroy`:

   > Attempting to destroy a condition variable upon which other threads are
   > currently blocked results in undefined behavior.

   When a process forks, it may have had a thread blocked on a condition
   variable.  Afte the fork, the condition variable is in a state where it
   think there's a thread blocked on it.  Even though there is no thread
   in the new process blocked, I suspect calling `pthread_cond_destroy` will
   still yield undefined behavior.

8. The `timeout` function in Figure 12.8 can be simplified substantially.
   Explain how.

   Skipped
