#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int
main(void)
{
	const pid_t pid = fork();
	if (pid < 0) {
		perror("fork");
		return 1;
	}

	if (pid == 0) {
		// Child terminates
		return 0;
	}

	// We could have registered for SIGCHLD and waited to receive that
	// signal, but for simplicity we'll just sleep "long enough" for
	// the child to start and terminate
	sleep(2);

	char buffer[80];
	int status = 0;

	snprintf(buffer, sizeof(buffer), "ps -p %d", pid);
	system(buffer);

	// This will reap the zombie.  If we didn't do this, when the process
	// terminated, the zombie would get re-parented to `init`, and `init`
	// would reap it.
	wait(&status);

	return 0;
}
