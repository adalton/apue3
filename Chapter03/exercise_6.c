/*
 * 6. If you open a file for read–write with the append flag, can you still read
 *    from anywhere in the file using `lseek`? Can you use `lseek` to replace
 *    existing data in the file? Write a program to verify this.
 */
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define BUFFER_SIZE 1024

int
main(const int argc, const char* argv[])
{
	int exit_status = 1;

	if (argc < 2) {
		fprintf(stderr, "usage: %s <filename>\n", argv[0]);
		return 1;
	}

	const int fd = open(argv[1], O_RDONLY | O_APPEND);
	if (fd < 0) {
		perror("open");
		return 1;
	}
	char buffer[BUFFER_SIZE] = {};

	if (lseek(fd, 0, SEEK_SET) < 0) {
		perror("lseek");
		goto done;
	}

	if (read(fd, buffer, sizeof(buffer) - 1) < 0) {
		perror("read");
		goto done;
	}
	printf("%s\n", buffer);

	if (write(fd, "Five", strlen("Five")) < 0) {
		perror("write");
		goto done;
	}

	exit_status = 0;

done:
	close(fd);

	return exit_status;
}
