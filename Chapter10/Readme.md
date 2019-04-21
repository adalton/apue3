1. In Figure 10.2, remove the `for (;;)` statement. What happens and why?

   The program pauses when `pause` is called.  If the program receives
   either `SIGUSR1` or `SIGUSR2`, it will fire the signal handler
   `sig_usr`. When the signal handler returns, `pause` returns and the
   program terminates normally.

   If the program receives any other signal, then the default signal
   handler for that signal will fire. That signal handler may terminate
   the program directly.  If not, again `pause` will return and the program
   will return normally.

2. Implement the `sig2str` function described in Section 10.22.

   Here is a possible implementation (also in `exercise_2.c`):

   ```c
   #include <signal.h>
   #include <stdio.h>
   #include <string.h>
   
   /* List of signals */
   #define SIGNALS(macro) \
   	macro(SIGHUP) \
   	macro(SIGINT) \
   	macro(SIGQUIT) \
   	macro(SIGILL) \
   	macro(SIGILL) \
   	macro(SIGTRAP) \
   	macro(SIGABRT) \
   	macro(SIGBUS) \
   	macro(SIGFPE) \
   	macro(SIGKILL) \
   	macro(SIGUSR1) \
   	macro(SIGSEGV) \
   	macro(SIGUSR2) \
   	macro(SIGPIPE) \
   	macro(SIGALRM) \
   	macro(SIGTERM) \
   	macro(SIGSTKFLT) \
   	macro(SIGCHLD) \
   	macro(SIGCONT) \
   	macro(SIGSTOP) \
   	macro(SIGTSTP) \
   	macro(SIGTTIN) \
   	macro(SIGTTOU) \
   	macro(SIGURG) \
   	macro(SIGXCPU) \
   	macro(SIGXFSZ) \
   	macro(SIGVTALRM) \
   	macro(SIGPROF) \
   	macro(SIGWINCH) \
   	macro(SIGIO) \
   	macro(SIGPWR) \
   	macro(SIGSYS)
   
   	/* I'll ignore real-time signals here for brevity */
   
   /* Map signal number to signal name */
   #define SIGNAL_ARRAY(sig) [sig] = #sig,
   
   static const char* const signal_names[] = {
   	SIGNALS(SIGNAL_ARRAY)
   };
   
   int
   sig2str(const int signo, char* const str)
   {
   	if (signo < 0 || signo > (sizeof(signal_names) / sizeof(signal_names[0]))) {
   		return -1;
   	}
   
   	if (signal_names[signo] == NULL) {
   		return -1;
   	}
   
   	strcpy(str, signal_names[signo] + strlen("SIG"));
   
   	return 0;
   }
   
   #define SIG2STR_MAX 10
   
   int
   main(void)
   {
   	char buffer[SIG2STR_MAX] = {};
   
   	if (sig2str(SIGTERM, buffer) < 0) {
   		fprintf(stderr, "sig2str failed\n");
   		return 1;
   	}
   
   	printf("%s\n", buffer);
   
   	return 0;
   }
   ```

3. Draw pictures of the stack frames when we run the program from Figure 10.9.

   See `10.3.pdf`.

4. In Figure 10.11, we showed a technique that's often used to set a timeout
   on an I/O operation using `setjmp` and `longjmp`. The following code has
   also been seen:

   ```c
   signal(SIGALRM, sig_alrm);
   alarm(60);
   if (setjmp(env_alrm) != 0) {
   	/* handle timeout */
   	...
   }
   ...
   ```
   What else is wrong with this sequence of code?

   By calling `alarm` before calling `setjmp`, we risk having `sig_alarm` fire
   before `setjmp` initializes `env_alarm`.  Consider a scenario where
   after `alarm` completes but before `setjmp` is called the process recives
   some other signal for which it has registered a signal handler.  If that
   handler takes more than 60 seconds, the kernel will deliver a `SIGALRM`
   signal to the process, and the process will execute `sig_alrm`.  The
   `sig_alrm` function will try to `longjmp` to `env_alrm`, but it has not
   yet been initialized.

5. Using only a single timer (either alarm or the higher-precision `setitimer`),
   provide a set of functions that allows a process to set any number of timers.

   I'm skipping the implementaiton of this one.  I'll instead describe the
   approach that I would take.
   
   I would define an API that enabled the process to register a logical
   timer. I would maintain each registered timer in a list, sorted such that
   the timer that will fire first is first in the list. I would register the
   "real" timer for that time.  When the timer fires, I'd run function
   registered for the timer.  I'd then sleep again for the delta between
   "now" and the next timer.

6. Write the following program to test the parent–child synchronization
   functions in Figure 10.24. The process creates a file and writes the
   integer 0 to the file. The process then calls `fork`, and the parent and
   child alternate incrementing the counter in the file. Each time the counter
   is incremented, print which process (parent or child) is doing the increment.

   Here is the implementation (also in `exercise_6.c`):

   ```c
   #include <signal.h>
   #include <stdio.h>
   #include <string.h>
   #include <sys/types.h>
   #include <unistd.h>
   
   static volatile sig_atomic_t sigflag; /* set nonzero by sig handler */
   static sigset_t newmask, oldmask, zeromask;
   
   static void
   sig_usr(int signo)  /* one signal handler for SIGUSR1 and SIGUSR2 */
   {
   	sigflag = 1;
   }
   
   static void
   TELL_WAIT(void)
   {
   	if (signal(SIGUSR1, sig_usr) == SIG_ERR)
   		perror("signal(SIGUSR1) error");
   
   	if (signal(SIGUSR2, sig_usr) == SIG_ERR)
   		perror("signal(SIGUSR2) error");
   
   	sigemptyset(&zeromask);
   	sigemptyset(&newmask);
   	sigaddset(&newmask, SIGUSR1);
   	sigaddset(&newmask, SIGUSR2);
   
   	/* Block SIGUSR1 and SIGUSR2, and save current signal mask */
   	if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0)
   		perror("SIG_BLOCK error");
   }
   
   static void
   TELL_PARENT(void)
   {
   	kill(getppid(), SIGUSR2); /* tell parent we’re done */
   }
   
   static void
   WAIT_PARENT(void)
   {
   	while (sigflag == 0)
   		sigsuspend(&zeromask);  /* and wait for parent */
   	sigflag = 0;
   	/* Reset signal mask to original value */
   	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
   		perror("SIG_SETMASK error");
   }
   
   static void
   TELL_CHILD(pid_t pid)
   {
       kill(pid, SIGUSR1); /* tell child we’re done */
   }
   
   static void
   WAIT_CHILD(void)
   {
   	while (sigflag == 0)
   		sigsuspend(&zeromask);  /* and wait for child */
   	sigflag = 0;
   	/* Reset signal mask to original value */
   	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
   		perror("SIG_SETMASK error");
   }
   
   static int
   increment_value(FILE* const file)
   {
   	int value = 0;
   
   	fseek(file, 0, SEEK_SET);
   	fread(&value, sizeof(value), 1, file);
   
   	++value;
   
   	fseek(file, 0, SEEK_SET);
   	fwrite(&value, sizeof(value), 1, file);
   	fflush(file);
   
   	return value;
   }
   
   int
   main(void)
   {
   	const int NUM_ITERATIONS = 100;
   
   	FILE* const file = fopen("/tmp/data", "w+");
   	if (file == NULL) {
   		perror("fopen");
   		return 1;
   	}
   
   	TELL_WAIT();
   	int i;
   	const pid_t pid = fork();
   
   	if (pid < 0) {
   		perror("fork");
   		return 1;
   	}
   
   	if (pid == 0) {
   		for (i = 0; i < NUM_ITERATIONS; ++i) {
   			printf(" child incrementing, value: %3d\n",
   			       increment_value(file));
   
   			TELL_PARENT();
   			WAIT_PARENT();
   		}
   	} else {
   		for (i = 0; i < NUM_ITERATIONS; ++i) {
   			WAIT_CHILD();
   
   			printf("parent incrementing, value: %3d\n",
   			       increment_value(file));
   
   			TELL_CHILD(pid);
   		}
   	}
   
	fclose(file);

   	return 0;
   }
   ```

   And here's the partial output:

   ```
   $ ./a.out
    child incrementing, value:   1
   parent incrementing, value:   2
    child incrementing, value:   3
   parent incrementing, value:   4
    child incrementing, value:   5
   parent incrementing, value:   6
   ...
    child incrementing, value: 195
   parent incrementing, value: 196
    child incrementing, value: 197
   parent incrementing, value: 198
    child incrementing, value: 199
   parent incrementing, value: 200
   ```

   And finally, the state of the file after program termination:

   ```
   $ hexdump --format '"%d\n"' /tmp/data
   200
   ```

7. In the function shown in Figure 10.25, if the caller catches `SIGABRT` and
   returns from the signal handler, why do we go to the trouble of resetting
   the disposition to its default and call kill the second time, instead of
   simply calling `_exit`?

8. Why do you think the `siginfo` structure (Section 10.14) includes the real
   user ID, instead of the effective user ID, in the `si_uid` field?

9. Rewrite the function in Figure 10.14 to handle all the signals from
   Figure 10.1. The function should consist of a single loop that iterates
   once for every signal in the current signal mask (not once for every
   possible signal).

10. Write a program that calls `sleep(60)` in an infinite loop. Every five
    times through the loop (every 5 minutes), fetch the current time of day
    and print the `tm_sec` field. Run the program overnight and explain the
    results. How would a program such as the `cron` daemon, which runs every
    minute on the minute, handle this situation?

11. Modify Figure 3.5 as follows: (a) change `BUFFSIZE` to 100; (b) catch the
    `SIGXFSZ` signal using the `signal_intr` function, printing a message when
    it's caught, and returning from the signal handler; and (c) print the
    return value from `write` if the requested number of bytes wasn't written.
    Modify the soft `RLIMIT_FSIZE` resource limit (Section 7.11) to 1,024 bytes
    and run your new program, copying a file that is larger than 1,024 bytes.
    (Try to set the soft resource limit from your shell. If you can't do this
    from your shell, call `setrlimit` directly from the program.) Run this
    program on the different systems that you have access to. What happens and
    why?

12. Write a program that calls `fwrite` with a large buffer (about one gigabyte).
    Before calling `fwrite`, call `alarm` to schedule a signal in 1 second. In
    your signal handler, print that the signal was caught and return. Does the
    call to `fwrite` complete? What’s happening?
