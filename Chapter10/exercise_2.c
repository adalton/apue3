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
