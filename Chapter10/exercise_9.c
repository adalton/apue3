#include <errno.h>
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

const char*
sig2str(const int signo)
{
	if (signo < 0 || signo > (sizeof(signal_names) / sizeof(signal_names[0]))) {
		return NULL;
	}

	if (signal_names[signo] == NULL) {
		return NULL;
	}

	return signal_names[signo];
}

void
pr_mask(const char* const str)
{
	const int errno_save = errno;
	sigset_t sigset = {};

	if (sigprocmask(0, NULL, &sigset) < 0) {
		/* we can be called by signal handlers */
		perror("sigprocmask");
		return;
	} else {
		int i;

		printf("%s", str);

		for (i = 1; i < NSIG; ++i) {
			if (sigismember(&sigset, i)) {
				const char* str = sig2str(i);

				if (str == NULL) {
					str = "Unknown";
				}

				printf(" %s", str);
			}
		}

		printf("\n");
	}
	errno = errno_save;     /* restore errno */
}

int
main(void)
{
	sigset_t sigset = {};

	sigemptyset(&sigset);
	sigaddset(&sigset, SIGUSR1);
	sigaddset(&sigset, SIGALRM);

	if (sigprocmask(SIG_BLOCK, &sigset, NULL) < 0) {
		perror("sigprocmask");
	}

	pr_mask("Blocked: ");

	return 0;
}
