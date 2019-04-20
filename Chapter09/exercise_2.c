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
		char buffer[128];

		if (setsid() < 0) {
			perror("setsid");
			return 1;
		}

		snprintf(buffer, sizeof(buffer), "ps -p %d -opid,cmd,pgrp,tpgid,session", getpid());
		system(buffer);
	} else {
		int status;

		// Wait for child to die
		wait(&status);
	}

	return 0;
}
