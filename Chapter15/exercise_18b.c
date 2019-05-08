#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#define NLOOPS      1000
#define SIZE        sizeof(long)

#define err_msg(fmt, ...) fprintf(stderr, (fmt "\n"), ##__VA_ARGS__)
#define err_sys(fmt, ...) fprintf(stderr, (fmt "\n"), ##__VA_ARGS__)
#define err_quit(fmt, ...) do { err_sys(fmt, ##__VA_ARGS__); exit(1); } while(0)


/* size of shared memory area */
static int
update(long *ptr)
{
    return((*ptr)++); /* return value before increment */
}

int
main(void)
{
	const char* const parent_semname = "/parent_semaphore";
	const char* const child_semname = "/child_semaphore";

	int fd, i, counter;
	pid_t pid;
	void *area;
	int sem_value;

	sem_t* const parent_sem = sem_open(parent_semname, O_CREAT, 0600, 1);
	sem_t* const child_sem  = sem_open(child_semname, O_CREAT, 0600, 0);
	
	if (parent_sem == SEM_FAILED || child_sem == SEM_FAILED) {
		perror("sem_open");
		return 1;
	}

	if ((fd = open("/dev/zero", O_RDWR)) < 0)
		err_sys("open error");

	if ((area = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
	                 fd, 0)) == MAP_FAILED)
		err_sys("mmap error");

	close(fd);      /* can close /dev/zero now that it’s mapped */

	if ((pid = fork()) < 0) {
		err_sys("fork error");
	} else if (pid > 0) {           /* parent */
		for (i = 0; i < NLOOPS; i += 2) {
			sem_wait(parent_sem);

			if ((counter = update((long *)area)) != i)
				err_quit("parent: expected %d, got %d", i, counter);

			sem_post(child_sem);
		}

		wait(NULL);
		sem_close(parent_sem);
		sem_close(child_sem);
		sem_unlink(parent_semname);
		sem_unlink(child_semname);
	} else {                        /* child */
		for (i = 1; i < NLOOPS + 1; i += 2) {
			sem_wait(child_sem);

			if ((counter = update((long *)area)) != i)
				err_quit("child: expected %d, got %d", i, counter);

			sem_post(parent_sem);
		}
	}
	exit(0); 
}
