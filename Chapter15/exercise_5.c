#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXLINE 4096

#define err_msg(fmt, ...) fprintf(stderr, (fmt "\n"), ##__VA_ARGS__)
#define err_sys(fmt, ...) fprintf(stderr, (fmt "\n"), ##__VA_ARGS__)
#define err_quit(fmt, ...) do { err_sys(fmt, ##__VA_ARGS__); exit(1); } while(0)

static void sig_pipe(int); /* our signal handler */

int
main(void)
{
	int     n, fd1[2], fd2[2];
	pid_t   pid;
	char    line[MAXLINE];

	if (signal(SIGPIPE, sig_pipe) == SIG_ERR)
		err_sys("signal error");

	if (pipe(fd1) < 0 || pipe(fd2) < 0)
		err_sys("pipe error");

	if ((pid = fork()) < 0) {
		err_sys("fork error");
	} else if (pid > 0) {
		close(fd1[0]);
		close(fd2[1]);

		/* vvv Added for exercise vvv */
		FILE* in = fdopen(fd2[0], "r");
		FILE* out = fdopen(fd1[1], "w");

		if (in == NULL) {
			err_sys("fdopen fd[1]");
		}

		if (out == NULL) {
			err_sys("fdopen fd[1]");
		}
		/* ^^^ Added for exercise ^^^ */

		/* parent */
		while (fgets(line, MAXLINE, stdin) != NULL) {
#if 0
                        /* vvv Removed for exercise vvv */
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
                        /* ^^^ Removed for exercise ^^^ */
#endif
			/* vvv Added for exercise vvv */
			fprintf(out, "%s", line);
			fflush(out);

			if (fgets(line, sizeof(line), in) == NULL) {
				err_msg("Failed to read from child");
				break;
			}
			/* ^^^ Added for exercise ^^^ */

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

static void
sig_pipe(int signo)
{
	printf("SIGPIPE caught\n");
	exit(1);
}
