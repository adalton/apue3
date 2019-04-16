#include <stdio.h>
#include <stdlib.h>

#define err_sys(fmt, ...) fprintf(stderr, "" fmt "\n", ##__VA_ARGS__)

#define MAXLINE 4

int
main(void)
{
	char buf[MAXLINE];

	while (fgets(buf, MAXLINE, stdin) != NULL)
		if (fputs(buf, stdout) == EOF)
			err_sys("output error");

	if (ferror(stdin))
		err_sys("input error");

	exit(0);
}
