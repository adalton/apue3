#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

static int
make_non_blocking(const int fd)
{
	const int flags = fcntl(fd, F_GETFL); 
	if (flags < 0) {
		perror("fcntl");
		return -1;
	}

	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int
main(void)
{
	int retval = 1;
	int pipe_fds[2] = {};

	if (pipe(pipe_fds) < 0) {
		perror("pipe");
		return 1;
	}

	// Make the write-end of the pipe non-blocking
	if (make_non_blocking(pipe_fds[1]) < 0) {
		goto close_pipe;
	}


	int i;
	for (i = 0; write(pipe_fds[1], "a", 1) == 1; ++i);

	printf("pipe size: %d\n", i);

	retval = 0;

close_pipe:
	close(pipe_fds[0]);
	close(pipe_fds[1]);

	return retval;
}
