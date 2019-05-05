#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int
main(void)
{
	int pipe_fds[2] = {};

	if (pipe(pipe_fds) < 0) {
		perror("pipe");
		return 1;
	}

	const pid_t pid = fork();
	if (pid < 0) {
		perror("fork");
		return 1;
	}

	if (pid > 0) {      /* parent */
		close(pipe_fds[0]);

		fd_set readfds;
		fd_set writefds;
		fd_set exceptfds;
		struct timeval* timeout = NULL;

		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		FD_ZERO(&exceptfds);

		FD_SET(pipe_fds[1], &writefds);

		const int fdcount = select(pipe_fds[1] + 1,
		                           &readfds,
		                           &writefds,
		                           &exceptfds,
		                           timeout);
		if (fdcount < 0) {
			perror("select");
			return 1;
		}

		printf("fdcount: %d\n", fdcount);
		printf("readfds set:   %d\n", FD_ISSET(pipe_fds[1], &readfds));
		printf("writefds set:  %d\n", FD_ISSET(pipe_fds[1], &writefds));
		printf("exceptfds set: %d\n", FD_ISSET(pipe_fds[1], &exceptfds));

		return 0;
	} else {            /* child */
		sleep(5); /* Let parent get blocked on call to select */

		/* These aren't strictly necessary since they'd be closed on exit */
		close(pipe_fds[0]);
		close(pipe_fds[1]);
		return 0;
	}

	return 0;
}
