#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int globvar = 6;

int
main(void)
{
	int var;
	pid_t pid;

	/* external variable in initialized data */
	/* automatic variable on the stack */
	var = 88;

	printf("before vfork\n");
	if ((pid = vfork()) < 0) {
		perror("vfork error");
	} else if (pid == 0) {
		globvar++;
		var++;
		/*_exit(0);*/
		fclose(stdout); /* simulate possible behavior */
		exit(0);
	}

	/* parent continues here */
	printf("pid = %ld, glob = %d, var = %d\n", (long)getpid(), globvar, var);
	exit(0);
}
