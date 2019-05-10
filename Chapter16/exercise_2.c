#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int
main(void)
{
	struct stat statbuf = {};
	const int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("socket");
		return 1;
	}

	if (fstat(fd, &statbuf) < 0) {
		perror("fstat");
		return 1;
	}

	printf("st_dev:     %lu\n", statbuf.st_dev);
	printf("st_ino:     %lu\n", statbuf.st_ino);
	printf("st_nlink:   %lu\n", statbuf.st_nlink);
	printf("st_mode:    %u\n",  statbuf.st_mode);
	printf("st_uid:     %u\n",  statbuf.st_uid);
	printf("st_gid:     %u\n",  statbuf.st_gid);
	printf("st_rdev:    %lu\n", statbuf.st_rdev);
	printf("st_size:    %lu\n", statbuf.st_size);
	printf("st_blksize: %lu\n", statbuf.st_blksize);
	printf("st_blocks:  %lu\n", statbuf.st_blocks);
	printf("st_atime:   %lu\n", statbuf.st_atime);
	printf("st_mtime:   %lu\n", statbuf.st_mtime);
	printf("st_ctime:   %lu\n", statbuf.st_ctime);

	close(fd);
	return 0;
}
