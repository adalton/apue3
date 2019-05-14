#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#define QLEN 10

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 256
#endif

#define err_sys(fmt, ...) fprintf(stderr, (fmt "\n"), ##__VA_ARGS__)
#define err_quit(fmt, ...) do { err_sys(fmt, ##__VA_ARGS__); exit(1); } while(0)

/* Track if we have artifically delayed the first client yet */
static int done_child_wait = 0;

static int initserver(int, const struct sockaddr *, socklen_t, int);
static void daemonize(const char *cmd);
static int set_cloexec(int fd);

void
serve(int sockfd)
{
	int clfd, status;
	pid_t pid;

	set_cloexec(sockfd);

	for (;;) {
		if ((clfd = accept(sockfd, NULL, NULL)) < 0) {
			syslog(LOG_ERR, "ruptimed: accept error: %s",
					strerror(errno));
			exit(1);
		}
		if ((pid = fork()) < 0) {
			syslog(LOG_ERR, "ruptimed: fork error: %s",
					strerror(errno));
			exit(1);
		} else if (pid == 0) {  /* child */
			/*
			 * The parent called daemonize (Figure 13.1), so
			 * STDIN_FILENO, STDOUT_FILENO, and STDERR_FILENO
			 * are already open to /dev/null.  Thus, the call to
			 * close doesn’t need to be protected by checks that
			 * clfd isn’t already equal to one of these values.
			 */
			if (dup2(clfd, STDOUT_FILENO) != STDOUT_FILENO ||
					dup2(clfd, STDERR_FILENO) != STDERR_FILENO) {
				syslog(LOG_ERR, "ruptimed: unexpected error");
				exit(1);
			}
			close(clfd);

			/* Artifically delay the first client */
			if(done_child_wait == 0) {
				sleep(10);
			}

			execl("/usr/bin/uptime", "uptime", (char *)0);
			syslog(LOG_ERR, "ruptimed: unexpected return from exec: %s",
					strerror(errno));
			exit(1);
		} else { /* parent */
			/* Don't delay any future clients */
			done_child_wait = 1;
			close(clfd);
		}
	}
}

static void
sigchld_handler(int signo)
{
	// Reap any children that have terminated
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

int
main(int argc, char *argv[])
{
	struct addrinfo *ailist, *aip;
	struct addrinfo hint;
	int             sockfd, err, n;
	char            *host;

	if (argc != 1)
		err_quit("usage: ruptimed");

	if (signal(SIGCHLD, sigchld_handler) < 0) {
		perror("signal");
		return 1;
	}

	if ((n = sysconf(_SC_HOST_NAME_MAX)) < 0)
		n = HOST_NAME_MAX;  /* best guess */

	if ((host = malloc(n)) == NULL)
		err_sys("malloc error");

	if (gethostname(host, n) < 0)
		err_sys("gethostname error");

	daemonize("ruptimed");

	memset(&hint, 0, sizeof(hint));
	hint.ai_flags = AI_CANONNAME;
	hint.ai_socktype = SOCK_STREAM;
	hint.ai_canonname = NULL;
	hint.ai_addr = NULL;
	hint.ai_next = NULL;

	if ((err = getaddrinfo(host, "ruptime", &hint, &ailist)) != 0) {
		syslog(LOG_ERR, "ruptimed: getaddrinfo error: %s",
				gai_strerror(err));
		exit(1);
	}
	for (aip = ailist; aip != NULL; aip = aip->ai_next) {
		if ((sockfd = initserver(SOCK_STREAM, aip->ai_addr,
		                         aip->ai_addrlen, QLEN)) >= 0) {
			serve(sockfd);
			exit(0);
		}
	}
	exit(1); 
}

static void
daemonize(const char *cmd)
{
	int i, fd0, fd1, fd2;
	pid_t pid;
	struct rlimit rl = {};
	struct sigaction sa = {};

	/*
	 * Clear file creation mask.
	 */
	umask(0);

	/*
	 * Get maximum number of file descriptors.
	 */
	if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
		err_quit("%s: can’t get file limit", cmd);

	/*
	 * Become a session leader to lose controlling TTY.
	 */
	if ((pid = fork()) < 0)
		err_quit("%s: can’t fork", cmd);
	else if (pid != 0) /* parent */
		exit(0);
	setsid();

	/*
	 * Ensure future opens won’t allocate controlling TTYs.
	 */
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);

	sa.sa_flags = 0;
	if (sigaction(SIGHUP, &sa, NULL) < 0)
		err_quit("%s: can’t ignore SIGHUP", cmd);
	if ((pid = fork()) < 0)
		err_quit("%s: can’t fork", cmd);
	else if (pid != 0) /* parent */
		exit(0);
	/*
	 * Change the current working directory to the root so
	 * we won’t prevent file systems from being unmounted.
	 */
	if (chdir("/") < 0)
		err_quit("%s: can’t change directory to /", cmd);
	/*
	 * Close all open file descriptors.
	 */
	if (rl.rlim_max == RLIM_INFINITY)
		rl.rlim_max = 1024;
	for (i = 0; i < rl.rlim_max; i++)
		close(i);

	/*
	 * Attach file descriptors 0, 1, and 2 to /dev/null.
	 */
	fd0 = open("/dev/null", O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);

	/*
	 * Initialize the log file.
	 */
	openlog(cmd, LOG_CONS, LOG_DAEMON);
	if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
		syslog(LOG_ERR, "unexpected file descriptors %d %d %d",
				fd0, fd1, fd2);
		exit(1);
	}
}

static int
initserver(int type, const struct sockaddr* addr, socklen_t alen, int qlen)
{
	int fd;
	int err = 0;

	if ((fd = socket(addr->sa_family, type, 0)) < 0) {
		return -1;
	}

	if (bind(fd, addr, alen) < 0) {
		goto errout;
	}

	if (type == SOCK_STREAM || type == SOCK_SEQPACKET) {
		if (listen(fd, qlen) < 0) {
			goto errout;
		}
	}
	return fd;

errout:
	err = errno;
	close(fd);
	errno = err;
	return -1;
}

static int
set_cloexec(int fd)
{
	int val;

	if ((val = fcntl(fd, F_GETFD, 0)) < 0) {
		return -1;
	}

	/* enable close-on-exec */
	return fcntl(fd, F_SETFD, val | FD_CLOEXEC);
}
