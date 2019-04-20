#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

static void charatatime(char *);

static volatile sig_atomic_t sigflag; /* set nonzero by sig handler */
static sigset_t newmask, oldmask, zeromask;

static void
sig_usr(int signo)      /* one signal handler for SIGUSR1 and SIGUSR2 */
{
	sigflag = 1;
}

void
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

void
TELL_CHILD(pid_t pid)
{
	kill(pid, SIGUSR1);                     /* tell child we're done */
}

void
WAIT_PARENT(void)
{
	while (sigflag == 0)
		sigsuspend(&zeromask);  /* and wait for parent */
	sigflag = 0;

	/* Reset signal mask to original value */
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
		perror("SIG_SETMASK error");
}

int
main(void)
{
	pid_t pid;

	TELL_WAIT();

	if ((pid = fork()) < 0) {
		perror("fork error");
	} else if (pid == 0) {
		WAIT_PARENT();          /* parent goes first */
		charatatime("output from child\n");
	} else {
		charatatime("output from parent\n");
		TELL_CHILD(pid);
	}
	exit(0);
}

static void
charatatime(char *str)
{
	char *ptr;
	int c;

	setbuf(stdout, NULL);                   /* set unbuffered */
	for (ptr = str; (c = *ptr++) != 0; )
		putc(c, stdout);
}
