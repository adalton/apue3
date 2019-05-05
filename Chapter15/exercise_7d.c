#define _GNU_SOURCE
#include <poll.h>
#include <stdio.h>
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

		struct pollfd pfd = {
			.fd = pipe_fds[1],
			.events = POLLOUT,
		};

		sleep(2); /* Let the child close the pipe first */

		const int fdcount = poll(&pfd, 1, /* infinite */ -1);
		if (fdcount < 0) {
			perror("poll");
			return 1;
		}

		printf("fdcount: %d\n", fdcount);
		printf("revents:");


		if (pfd.revents & POLLIN)     { printf(" POLLIN");      }
		if (pfd.revents & POLLPRI)    { printf(" POLLPRI");     }
		if (pfd.revents & POLLOUT)    { printf(" POLLOUT");     }
#if defined(_GNU_SOURCE)
		if (pfd.revents & POLLRDHUP)  { printf(" POLLRDHUP");   }
#endif
		if (pfd.revents & POLLERR)    { printf(" POLLERR");     }
		if (pfd.revents & POLLHUP)    { printf(" POLLHUP");     }
		if (pfd.revents & POLLNVAL)   { printf(" POLLNVAL");    }
#if defined(_XOPEN_SOURCE)
		if (pfd.revents & POLLRDNORM) { printf(" POLLRDNORM");  }
		if (pfd.revents & POLLRDBAND) { printf(" POLLRDBAND");  }
		if (pfd.revents & POLLWRNORM) { printf(" POLLWRNORM");  }
		if (pfd.revents & POLLWRBAND) { printf(" POLLWRBAND");  }
#endif
		printf("\n");

		return 0;
	} else {            /* child */
		/* These aren't strictly necessary since they'd be closed on exit */
		close(pipe_fds[0]);
		close(pipe_fds[1]);
		return 0;
	}

	return 0;
}
