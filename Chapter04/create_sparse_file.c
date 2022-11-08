#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define FOUR_GB (1024UL * 1024UL * 1024UL * 4UL)

int
main(const int argc, const char* const argv[])
{
	int exit_status = 1;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		return 1;
	}

	const int fd = open(argv[1], O_CREAT | O_WRONLY, 0644);
	if (fd < 0) {
		perror("open");
		return 1;
	}

	if (write(fd, "hi", strlen("hi")) < 0) {
		perror("write");
		goto done;
	}

	if (lseek(fd, FOUR_GB, SEEK_CUR) < 0) {
		perror("lseek");
		goto done;
	}

	if (write(fd, "ho", strlen("ho")) < 0) {
		perror("write");
		goto done;
	}

	exit_status = 0;

done:
	close(fd);
	return exit_status;
}
