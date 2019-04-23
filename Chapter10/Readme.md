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

   If we called `_exit`, then the exit status of the process would not indicate
   that the process was terminated because of a signal.

8. Why do you think the `siginfo` structure (Section 10.14) includes the real
   user ID, instead of the effective user ID, in the `si_uid` field?

   For the motivation behind this decision, consider the question "who can
   send a signal to a process?"  The answer is generally "the owner of the
   process" and "root".  Given this, if the field included the effective user
   ID, then the only possible values would be the UID of the owner or
   the UID of root (0).

   By including the real user id, then the value can be the UID of the owner
   or the UID of any user who can gain root privileges.  If there are
   multiple such users, including the real user ID enables the program to
   identify who really sent the signal.
  
9. Rewrite the function in Figure 10.14 to handle all the signals from
   Figure 10.1. The function should consist of a single loop that iterates
   once for every signal in the current signal mask (not once for every
   possible signal).

   I do not see any way to iterate over the contents of a `sigset_t`, so I
   cannot think of a way to loop once for each signal in the signal mask.

   I have an implementation that loops over all signals in `exercise_9.c`,
   that uses the function I developed for Exercise 2.

10. Write a program that calls `sleep(60)` in an infinite loop. Every five
    times through the loop (every 5 minutes), fetch the current time of day
    and print the `tm_sec` field. Run the program overnight and explain the
    results. How would a program such as the `cron` daemon, which runs every
    minute on the minute, handle this situation?

    Interestingly, I ran this program for almost 24 hours (279 5-minute
    samples) and the minute value did not change; the program is here
    and can be found in `exercise_10.c`:

    ```c
    #include <stdio.h>
    #include <sys/time.h>
    #include <time.h>
    #include <unistd.h>
    
    int main(void)
    {
    	int i;
    	for (i = 0;; ++i) {
    		time_t current_time;
    
    		time(&current_time);
    		const struct tm* const now = gmtime(&current_time);
    
    		if (i % 5 == 0) {
    			printf("%d\n", now->tm_sec);
    			fflush(stdout);
    		}
    
    		sleep(60);
    	}
    }
    ```

    Given the question, I suspect that the author anticipated that the seconds
    would drift (increase) over time.  How would something like `cron` deal
    with drifting?  Since it runs every minute, it could maintain information
    about the last time it fired and adjust the sleep value accordingly.  If
    the time drifts by a second, then next time it can sleep for only 59
    seconds.

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

    Here's the program (also in `exercise_11.c`):

    ```c
    #include <signal.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <sys/resource.h>
    #include <sys/time.h>
    #include <unistd.h>
    
    typedef void (Sigfunc)(int);
    
    static Sigfunc*
    signal_intr(int signo, Sigfunc *func)
    {
    	struct sigaction act;
    	struct sigaction oact;
    
    	act.sa_handler = func;
    
    	sigemptyset(&act.sa_mask);
    	act.sa_flags = 0;
    
    #ifdef  SA_INTERRUPT
    	act.sa_flags |= SA_INTERRUPT;
    #endif
    	if (sigaction(signo, &act, &oact) < 0) {
    		return(SIG_ERR);
    	}
    
    	return(oact.sa_handler);
    }
    
    static void
    sig_handler(const int signo)
    {
    #define MESSAGE_TEXT "signal received\n"
    	write(STDERR_FILENO, MESSAGE_TEXT, sizeof(MESSAGE_TEXT) - 1);
    #undef MESSAGE_TEXT
    }
    
    static void
    set_limit(void)
    {
    	struct rlimit limit = {};
    
    	if (getrlimit(RLIMIT_FSIZE, &limit) < 0) {
    		perror("getrlimit");
    		return;
    	}
    
    	limit.rlim_cur = 1024;
    
    	if (setrlimit(RLIMIT_FSIZE, &limit) < 0) {
    		perror("setrlimit");
    		return;
    	}
    }
    
    #define BUFFSIZE 100
    
    int
    main(void)
    {
    	int n;
    	char buf[BUFFSIZE];
    
    	set_limit();
    	signal_intr(SIGXFSZ, sig_handler);
    
    	while ((n = read(STDIN_FILENO, buf, BUFFSIZE)) > 0) {
    		const int bytes_written = write(STDOUT_FILENO, buf, n);
    		if (bytes_written != n) {
    			fprintf(stderr,
    			        "write: expected/received: %d/%d\n",
    			        n,
    			        bytes_written);
    
    			if (bytes_written < 0) {
    				perror("write");
    				exit(1);
    			}
    		}
    	}
    
    	if (n < 0) {
    		perror("read");
    		exit(1);
    	}
    	exit(0);
    }
    ```

    Note that setting the limit in the shell didn't have and effect, so
    instead I implemented to change in the program itself.

    Here's a sample run of the program.  I ran this on Linux 4.19.27 and
    MacOS Mojave 18.5.0; I got the same results in both cases:

    ```
    $ dd if=/dev/urandom of=./random bs=1124 count=1
    $ cat ./random | ./a.out > ./random.out
    write: expected/received: 100/24
    signal received
    write: expected/received: 24/-1
    write: File too large
    $ ls -l random random.out
    -rw-r----- 1 user group 1124 Apr 22 21:48 random
    -rw-r----- 1 user group 1024 Apr 22 21:49 random.out
    ```

    The `read`/`write` loop continued for the first 1000 bytes, reading and
    writing 100 bytes at a time.  Next, the program successfully read the next
    100 bytes (1000-1100), and tried to write those 100 bytes. It was able to
    write only the first 24 bytes (reaching the file size limit of 1024).
    Bytes 25-100 (1025-1100) were dropped.  Next, the program read the final
    24 bytes (1101-1124) from the input file, and tried to write those to the
    output file.  This call to `write` triggered a `SIGXFSZ` signal.  The
    `write` system call then returned -1 with `errno` "File too large".

12. Write a program that calls `fwrite` with a large buffer (about one gigabyte).
    Before calling `fwrite`, call `alarm` to schedule a signal in 1 second. In
    your signal handler, print that the signal was caught and return. Does the
    call to `fwrite` complete? What’s happening?

    Here's the program:

    ```c
    #include <signal.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    
    static void
    sig_handler(const int signo)
    {
    #define MESSAGE_TEXT "signal received\n"
    	write(STDERR_FILENO, MESSAGE_TEXT, sizeof(MESSAGE_TEXT) - 1);
    #undef MESSAGE_TEXT
    }
    
    #define BUFFER_SIZE (1024 * 1024 * 1024)
    
    int
    main(void)
    {
    	// Allocate a 1GB buffer
    	char* const buffer = calloc(sizeof(char), BUFFER_SIZE);
    	if (buffer == NULL) {
    		perror("calloc");
    		return 1;
    	}
    
    	FILE* const file = fopen("/tmp/ex12.out", "w");
    	if (file == NULL) {
    		perror("fopen");
    		free(buffer);
    		return 1;
    	}
    
    	signal(SIGALRM, sig_handler);
    	alarm(1);
    
    	const size_t written = fwrite(buffer, BUFFER_SIZE, 1, file);
    	if (written != 1) {
    		fprintf(stderr, "Failed to write buffer\n");
    	}
    
    	fclose(file);
    	free(buffer);
    
    	return 0;
    }
    ```

    Here's a sample run of the program:

    ```
    $ date ; ./a.out ; date
    Mon Apr 22 22:41:25 EDT 2019
    signal received
    Mon Apr 22 22:41:36 EDT 2019
    ```

    On Linux I notice that the "signal received" message is printed immediately
    before the program terminates.  Using `sysdig`, I verified this:

    ```
    $ sudo sysdig proc.name=a.out
    ...
    20934 22:49:00.322959018 0 a.out (10622) < openat fd=3(<f>/tmp/ex12.out) dirfd=-100(AT_FDCWD) name=/tmp/ex12.out flags=262(O_TRUNC|O_CREAT|O_WRONLY) mode=0666
    20943 22:49:00.323339126 0 a.out (10622) > rt_sigaction
    20944 22:49:00.323341564 0 a.out (10622) < rt_sigaction
    20945 22:49:00.323367667 0 a.out (10622) > alarm
    20946 22:49:00.323370819 0 a.out (10622) < alarm
    20950 22:49:00.323483213 0 a.out (10622) > fstat fd=3(<f>/tmp/ex12.out)
    20951 22:49:00.323485717 0 a.out (10622) < fstat res=0
    20952 22:49:00.323491742 0 a.out (10622) > write fd=3(<f>/tmp/ex12.out) size=1073741824
    313635 22:49:10.883562159 0 a.out (10622) < write res=1073741824 data=...
    313636 22:49:10.883566887 0 a.out (10622) > signaldeliver spid=0 dpid=10622(a.out) sig=14(SIGALRM)
    314275 22:49:10.888009194 0 a.out (10622) > write fd=2(<f>/dev/pts/1) size=16
    314276 22:49:10.888020665 0 a.out (10622) < write res=16 data=signal received.
    314280 22:49:10.888064195 0 a.out (10622) > sigreturn
    ```

    Notice that the `write` to `/tmp/ex12.out` starts at `22:49:00`, finishes
    at `22:49:10`, and that the signal is delivered immediately after the
    `write` completes.  It looks like the call to `write` was uninterruptible;
    the kernel was able to trigger the process to execute the signal handler
    only after `write` returned.

    To verify this, I re-ran the program and used the `ps` command to examine
    its process state:

    ```
    $ ps aux | grep a.out
    user   1193 66.0 88.0 1052940 889432 pts/1  D+   23:32   0:00 ./a.out
    ```

    Notice the `D+` process state.  According to the man page for `ps`, the
    `D` process state code means "uninterruptible sleep (usually IO)"
    (the `+` means the process is in the foreground process group).  This
    explains why the signal was received by the process only after `write`
    completed.

    On MacOS I see different behavior from this application: the "signal
    received" message appears about a second after starting the program.  That
    said, examining the generated file I notice that it is the correct size
    (1GB).  For MacOS it look like `fwrite` handles the interrupted `write`
    system call by using the return value (and `errno`) to pick up at the
    point where the system call was interrupted.  The `fwrite` function isolates
    the caller from the underlying details of having signals interrupt the
    system calls.
