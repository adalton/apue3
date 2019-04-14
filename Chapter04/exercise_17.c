#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

int
main(void)
{
	const char* const path = "/dev/fd/1";

	if (unlink(path) < 0) {
		perror("unlink");
	}

	const int fd = creat(path, 0644);
	if (fd < 0) {
		perror("creat");
	}

	return 0;
}
