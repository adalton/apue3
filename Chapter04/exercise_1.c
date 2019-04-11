#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int
main(const int argc, const char *argv[])
{
	int i;

	for (i = 1; i < argc; i++) {
		struct stat buf = {};
		const char *ptr;

		printf("%s: ", argv[i]);
		if (stat(argv[i], &buf) < 0) {
			perror("lstat error");
			continue;
		}

		if (S_ISREG(buf.st_mode))
			ptr = "regular";
		else if (S_ISDIR(buf.st_mode))
			ptr = "directory";
		else if (S_ISCHR(buf.st_mode))
			ptr = "character special";
		else if (S_ISBLK(buf.st_mode))
			ptr = "block special";
		else if (S_ISFIFO(buf.st_mode))
			ptr = "fifo";
		else if (S_ISLNK(buf.st_mode))
			ptr = "symbolic link";
		else if (S_ISSOCK(buf.st_mode))
			ptr = "socket";
		else
			ptr = "** unknown mode **";

		printf("%s\n", ptr);
	}

	exit(0);
}
