#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

static int
rw_fifo(int fifo_fds[2], const char* const fifo)
{
	if (mkfifo(fifo, 0600) < 0) {
		if (errno != EEXIST) {
			perror("mkfifo");
			return -1;
		}
	}

	/* Open read-end non-blocking */
	fifo_fds[0] = open(fifo, O_RDONLY | O_NONBLOCK);
	if (fifo_fds[0] < 0) {
		perror("open");
		return -1;
	}

	/* Open write-end */
	fifo_fds[1] = open(fifo, O_WRONLY);
	if (fifo_fds[1] < 0) {
		perror("open");
		close(fifo_fds[0]);
		return -1;
	}

	/* Get the flags for the read-end */
	const int flags = fcntl(fifo_fds[0], F_GETFL);
	if (flags < 0) {
		perror("fcntl(F_GETFL)");
		close(fifo_fds[0]);
		close(fifo_fds[1]);
		return -1;
	}

	/* Make the read-end blocking */
	if (fcntl(fifo_fds[0], F_SETFL, flags & ~O_NONBLOCK) < 0) {
		perror("fcntl(F_SETFL)");
		close(fifo_fds[0]);
		close(fifo_fds[1]);
		return -1;
	}

	return 0;
}

int
main(void)
{
	const char* const fifo = "/tmp/exercise_15.10.fifo";
	int fifo_fds[2] = {};

	if (rw_fifo(fifo_fds, fifo) < 0) {
		return 1;
	}

	const pid_t pid = fork();

	if (pid < 0) {
		perror("fork");
		return 1;
	}

	if (pid == 0) {                                        /* child */
		const char msg[] = "Hello, world!";
		close(fifo_fds[0]);

		write(fifo_fds[1], msg, sizeof(msg) - 1);
	} else {                                               /* parent */
		char line[128] = {};

		close(fifo_fds[1]);

		read(fifo_fds[0], line, sizeof(line) - 1);
		printf("%s\n", line);

		wait(NULL);
	}

	return 0;
}
