#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <unistd.h>

#define NLOOPS      1000
#define SIZE        sizeof(long)

#define err_msg(fmt, ...) fprintf(stderr, (fmt "\n"), ##__VA_ARGS__)
#define err_sys(fmt, ...) fprintf(stderr, (fmt "\n"), ##__VA_ARGS__)
#define err_quit(fmt, ...) do { err_sys(fmt, ##__VA_ARGS__); exit(1); } while(0)

static volatile sig_atomic_t sigflag; /* set nonzero by sig handler */
static sigset_t newmask, oldmask, zeromask;

static void
sig_usr(int signo)  /* one signal handler for SIGUSR1 and SIGUSR2 */
{
	sigflag = 1;
}

static void
TELL_WAIT(void)
{
	if (signal(SIGUSR1, sig_usr) == SIG_ERR)
		perror("signal(SIGUSR1) error");

	if (signal(SIGUSR2, sig_usr) == SIG_ERR)
		perror("signal(SIGUSR2) error");

	sigemptyset(&zeromask);
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGUSR1);
	sigaddset(&newmask, SIGUSR2);

	/* Block SIGUSR1 and SIGUSR2, and save current signal mask */
	if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0)
		perror("SIG_BLOCK error");
}

static void
TELL_PARENT(void)
{
	kill(getppid(), SIGUSR2); /* tell parent we’re done */
}

static void
WAIT_PARENT(void)
{
	while (sigflag == 0)
		sigsuspend(&zeromask);  /* and wait for parent */
	sigflag = 0;
	/* Reset signal mask to original value */
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
		perror("SIG_SETMASK error");
}

static void
TELL_CHILD(pid_t pid)
{
    kill(pid, SIGUSR1); /* tell child we’re done */
}

static void
WAIT_CHILD(void)
{
	while (sigflag == 0)
		sigsuspend(&zeromask);  /* and wait for child */
	sigflag = 0;
	/* Reset signal mask to original value */
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
		perror("SIG_SETMASK error");
}

/* size of shared memory area */
static int
update(long *ptr)
{
    return((*ptr)++); /* return value before increment */
}

int
main(void)
{
	int i, counter;
	pid_t pid;
	void *area;

	/* picked /etc/passwd because it exists; normally I'd use something else */
	const key_t key = ftok("/etc/passwd", 0);
	if (key < 0) {
		perror("ftok");
		return 1;
	}

	/* Create a shared memory segment */
	const int shm_id = shmget(key, SIZE, IPC_CREAT | 0600);
	if (shm_id < 0) {
		perror("shmget");
		return 1;
	}

	/* Map the shared memory segment into our address space. */
	area = shmat(shm_id, NULL, 0);
	if (area == NULL) {
		perror("shmat");
		return 1;
	}

	TELL_WAIT();

	if ((pid = fork()) < 0) {
		err_sys("fork error");
	} else if (pid > 0) {           /* parent */
		for (i = 0; i < NLOOPS; i += 2) {
			if ((counter = update((long *)area)) != i)
				err_quit("parent: expected %d, got %d", i, counter);
			printf("p: %ld\n", *((long*) area));
			TELL_CHILD(pid);
			WAIT_CHILD();
		}

		wait(NULL);
	} else {                        /* child */
		for (i = 1; i < NLOOPS + 1; i += 2) {
			WAIT_PARENT();
			if ((counter = update((long *)area)) != i)
				err_quit("child: expected %d, got %d", i, counter);
			printf("c: %ld\n", *((long*) area));
			TELL_PARENT();
		}
		exit(0); 
	}

	/* Unmap the shared memory segment from our address space. */
	if (shmdt(area) < 0) {
		perror("shmdt");
		return 1;
	}

	/* Delete the shared memory segment */
	if (shmctl(shm_id, IPC_RMID, NULL) < 0) {
		perror("shmctl");
		return 1;
	}

	exit(0); 
}
