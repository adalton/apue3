#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXLINE 4096

#define err_msg(fmt, ...) fprintf(stderr, (fmt "\n"), ##__VA_ARGS__)
#define err_sys(fmt, ...) fprintf(stderr, (fmt "\n"), ##__VA_ARGS__)
#define err_quit(fmt, ...) do { err_sys(fmt, ##__VA_ARGS__); exit(1); } while(0)

/* exercise r -- static void sig_pipe(int); -- */ /* our signal handler */

int
main(void)
{
	int     n, fd1[2], fd2[2];
	pid_t   pid;
	char    line[MAXLINE];

	/** exercise 4 --
	if (signal(SIGPIPE, sig_pipe) == SIG_ERR)
		err_sys("signal error");
	-- */

	if (pipe(fd1) < 0 || pipe(fd2) < 0)
		err_sys("pipe error");

	if ((pid = fork()) < 0) {
		err_sys("fork error");
	} else if (pid > 0) {
		close(fd1[0]);
		close(fd2[1]);

		/* parent */
		while (fgets(line, MAXLINE, stdin) != NULL) {
			n = strlen(line);
			if (write(fd1[1], line, n) != n)
				err_sys("write error to pipe");

			if ((n = read(fd2[0], line, MAXLINE)) < 0)
				err_sys("read error from pipe");

			if (n == 0) {
				err_msg("child closed pipe");
				break;
			}
			line[n] = 0;    /* null terminate */
			if (fputs(line, stdout) == EOF)
				err_sys("fputs error");
		}
		if (ferror(stdin))
			err_sys("fgets error on stdin");
		exit(0);
	} else {
		close(fd1[1]);
		close(fd2[0]);
		if (fd1[0] != STDIN_FILENO) {
			/* child */
			if (dup2(fd1[0], STDIN_FILENO) != STDIN_FILENO)
				err_sys("dup2 error to stdin");
			close(fd1[0]);
		}
		if (fd2[1] != STDOUT_FILENO) {
			if (dup2(fd2[1], STDOUT_FILENO) != STDOUT_FILENO)
				err_sys("dup2 error to stdout");
			close(fd2[1]);
		}
		if (execl("./add2", "add2", (char *)0) < 0)
			err_sys("execl error");
	}
	exit(0);
}

/** exercise 4 --
static void
sig_pipe(int signo)
{
	printf("SIGPIPE caught\n");
	exit(1);
}
-- */
