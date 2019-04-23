/*
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
*/
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
