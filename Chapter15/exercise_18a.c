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
	int fd, i, counter;
	pid_t pid;
	void *area;
	int sem_value;

	// Note: In this version I don't actually use the semaphore as a
	//       semaphore; it's really just a shared variable that implements
	//       strict alternation.  This implementation is _very_ slow
	//       compared to my second implementation (see exercise_18b.c).
	sem_t* const semaphore = sem_open("/semaphore", O_CREAT | 0600, 0);
	
	if (semaphore == SEM_FAILED) {
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
			do { /* Busy wait for our turn */
				if (sem_getvalue(semaphore, &sem_value) < 0) {
					perror("sem_getvalue");
					return 1;
				}
			} while (sem_value != 0);

			if ((counter = update((long *)area)) != i)
				err_quit("parent: expected %d, got %d", i, counter);
			if (sem_post(semaphore) < 0) {
				perror("sem_post");
				exit(1);
			}
		}

		wait(NULL);
		sem_close(semaphore);
		sem_unlink("/semaphore");
	} else {                        /* child */
		for (i = 1; i < NLOOPS + 1; i += 2) {
			do { /* Busy wait for our turn */
				if (sem_getvalue(semaphore, &sem_value) < 0) {
					perror("sem_getvalue");
					return 1;
				}
			} while (sem_value == 0);

			if ((counter = update((long *)area)) != i)
				err_quit("child: expected %d, got %d", i, counter);
			if (sem_wait(semaphore) < 0) {
				perror("sem_post");
				exit(1);
			}
		}
	}
	exit(0); 
}
