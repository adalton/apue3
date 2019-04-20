#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

static int
test_opendir(const char* const directory)
{
	int retval = 1;
	int flags = 0;
	DIR* const root = opendir("/");

	if (root == NULL) {
		perror("opendir");
		return 1;
	}

	const int root_fd = dirfd(root);
	if (root_fd < 0) {
		perror("dirfd");
		goto cleanup;
	}

	flags = fcntl(root_fd, F_GETFD);
	if (flags < 0) {
		perror("fcntl");
		goto cleanup;
	}

	printf("Close-on-exec set via opendir(): %s\n",
	       (flags & FD_CLOEXEC) ? "true" : "false");

	retval = 0;

cleanup:
	closedir(root);
	return retval;
}

static int
test_open(const char* const directory)
{
	int retval = 1;
	const int root_fd = open(directory, O_RDONLY);
	if (root_fd < 0) {
		perror("open");
		return 1;
	}

	const int flags = fcntl(root_fd, F_GETFD);
	if (flags < 0) {
		perror("fcntl");
		goto cleanup;
	}

	printf("Close-on-exec set via open(): %s\n",
	       (flags & FD_CLOEXEC) ? "true" : "false");

	retval = 0;

cleanup:
	close(root_fd);
	return retval;
}

int
main(void)
{
	const char* const directory = "/";

	if (test_opendir(directory) != 0) {
		return 1;
	}

	if (test_open(directory) != 0) {
		return 1;
	}
}
