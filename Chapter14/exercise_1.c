/*
1. Write a test program that illustrates your system’s behavior when a process
   is blocked while trying to write lock a range of a file and additional
   read-lock requests are made. Is the process requesting a write lock starved
   by the processes read locking the file?
*/
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static int
lock(const int fd, const int lock_type)
{
	struct flock fl = {};

	fl.l_type = lock_type;
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 0; /* lock entire file */

	if (fcntl(fd, F_SETLKW, &fl) < 0) {
		perror("fcntl");
		return 1;
	}

	return 0;
}

static int
unlock(const int fd)
{
	struct flock fl = {};

	fl.l_type = F_UNLCK;

	if (fcntl(fd, F_SETLKW, &fl) < 0) {
		perror("fcntl");
		return 1;
	}

	return 0;
}

static int
reader(const char* const filename, const int reader_num, const int sleep_time)
{
	const int fd = open(filename, O_RDONLY);
	int i;

	if (fd < 0) {
		perror("open");
		return 1;
	}

	fprintf(stderr, "\nreader %d attempting to acquire read lock", reader_num);
	lock(fd, F_RDLCK);
	fprintf(stderr, "\nreader %d acquired read lock", reader_num);

	sleep(sleep_time);

	unlock(fd);
	fprintf(stderr, "\nreader %d unlocked", reader_num);

	return 0;
}

static int
writer(const char* const filename, const int sleep_time)
{
	struct stat statbuf = {};

	if (stat(filename, &statbuf) < 0) {
		perror("stat");
		return 1;
	}

	const int fd = open(filename, O_WRONLY);
	if (fd < 0) {
		perror("open");
		return 1;
	}

	fprintf(stderr, "\nwriter attempting to acquire write lock");
	lock(fd, F_WRLCK);
	fprintf(stderr, "\nwriter acquired write lock");

	sleep(sleep_time);

	unlock(fd);
	fprintf(stderr, "\nwriter unlocked");

	return 0;
}

int
main(const int argc, const char* const argv[])
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s <filename>\n", argv[0]);
		return 1;
	}

	const char* const filename = argv[1];

	srand(time(NULL));

	/* Child1 will get and hold a read lock */
	const pid_t child1 = fork();
	if (child1 < 0) {
		perror("fork");
		return 1;
	} else if (child1 == 0) {
		return reader(filename, 1, 15);
	}

	/* Make sure child1 starts and gets the write lock */
	sleep(5);

	/* Child 2 will try to get a write lock while child1 still holds
	   the read lock */
	const pid_t child2 = fork();
	if (child2 < 0) {
		perror("fork");
		return 1;
	} else if (child2 == 0) {
		return writer(filename, 2);
	}

	/* Make sure child2 starts and blocks on the write lock */
	sleep(5);

	/* Child3 will start with child2 blocked trying to acquire the
	   write lock.  Will it prevent child2 from running? */
	const pid_t child3 = fork();
	if (child3 < 0) {
		perror("fork");
		return 1;
	} else if (child3 == 0) {
		return reader(filename, 2, 20);
	}

	/* Wait for and clean up all the children */
	wait(NULL);
	wait(NULL);
	wait(NULL);

	printf("\n");
	return 0;
}
