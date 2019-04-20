#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void
sig_int(int signo)
{
	volatile int k = 0;

	/*
	 * Tune these loops to run for more than 5 seconds
	 * on whatever system this test program is run.
	 */
	printf("\nsig_int starting\n");
	int i;
	for (i = 0; i < 300000; i++) {
		int j;
		for (j = 0; j < 12000; j++) {
			k += i * j;
		}
	}

	printf("sig_int finished\n");
}

static jmp_buf env_alrm;

static void
sig_alrm(int signo)
{
	longjmp(env_alrm, 1);
}

unsigned int
sleep2(unsigned int seconds)
{
	if (signal(SIGALRM, sig_alrm) == SIG_ERR) {
		return(seconds);
	}

	if (setjmp(env_alrm) == 0) {
		alarm(seconds);         /* start the timer */
		pause();                /* next caught signal wakes us up */
	}
	return(alarm(0));               /* turn off timer, return unslept time */
}

int
main(void)
{
	unsigned int unslept;

	if (signal(SIGINT, sig_int) == SIG_ERR) {
		perror("signal(SIGINT) error");
	}
	unslept = sleep2(5);
	printf("sleep2 returned: %u\n", unslept);

	exit(0);
}

