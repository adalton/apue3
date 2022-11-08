#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* This will be on the stack, so don't make it too big. */
#define BUFFER_SIZE (4096)

static int
cp(const char* const source, const char* const dest)
{
	int exit_status = 1;
	const int source_fd = open(source, O_RDONLY);

	if (source_fd < 0) {
		perror("open(source)");
		goto out;
	}

	// We could use stat here to get the existing mode of source and
	// duplicate that, but I'm focused on creating the sparse file and
	// not the permissions.
	const int dest_fd = open(dest, O_CREAT | O_WRONLY, 0644);

	if (dest_fd < 0) {
		perror("open(dest)");
		goto close_source;
	}

	char buffer[BUFFER_SIZE];
	int bytes_read;

	while ((bytes_read = read(source_fd, buffer, sizeof(buffer))) > 0) {
		int bytes_scanned = 0;
		int bytes_written = 0;

		while (bytes_written < bytes_read) {
			int non_zero_count = 0;

			while ((bytes_scanned < bytes_read) &&
			       (buffer[bytes_scanned] != '\0')) {
				++non_zero_count;
				++bytes_scanned;
			}
			if (write(dest_fd, buffer + bytes_written, non_zero_count) < 0) {
				perror("write");
				goto close_dest;
			}
			bytes_written += non_zero_count;

			int zero_count = 0;
			while ((bytes_scanned < bytes_read) &&
			       (buffer[bytes_scanned] == '\0')) {
				++zero_count;
				++bytes_scanned;
			}

			//write(dest_fd, buffer + bytes_written, zero_count);
			const off_t current = lseek(dest_fd, zero_count, SEEK_CUR);
			if (current < 0) {
				perror("lseek");
				goto close_dest;
			}
			bytes_written += zero_count;
		}
	}

	if (bytes_read < 0) {
		perror("read");
		goto close_dest;
	}

	// All good -- we finished successfully
	exit_status = 0;

close_dest:
	close(dest_fd);

close_source:
	close(source_fd);

out:
	return exit_status;
}

int
main(const int argc, const char* const argv[])
{
	int exit_status = 1;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <source> <dest>\n", argv[0]);
		return 1;
	}

	const char* const source = argv[1];
	const char* const dest = argv[2];

	if (strcmp(source, dest) == 0) {
		fprintf(stderr, "<source> cannot be the same as <dest>\n");
		return 1;
	}

	return cp(source, dest);
}
