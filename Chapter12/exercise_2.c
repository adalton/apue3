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
