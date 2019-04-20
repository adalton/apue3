/*
3. Rewrite the program in Figure 8.6 to use `waitid` instead of `wait`.  Instead
   of calling `pr_exit`, determine the equivalent information from the
   `signinfo` structure.

       int waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options);

       Upon successful return, waitid() fills in the following fields  of  the  siginfo_t
       structure pointed to by infop:

       si_pid      The process ID of the child.

       si_uid      The  real  user ID of the child.  (This field is not set on most other
                   implementations.)

       si_signo    Always set to SIGCHLD.

       si_status   Either the exit  status  of  the  child,  as  given  to  _exit(2)  (or
                   exit(3)),  or  the signal that caused the child to terminate, stop, or
                   continue.  The si_code field can be used to determine how to interpret
                   this field.

       si_code     Set  to  one of: CLD_EXITED (child called _exit(2)); CLD_KILLED (child
                   killed by signal); CLD_DUMPED (child  killed  by  signal,  and  dumped
                   core);  CLD_STOPPED  (child  stopped  by  signal); CLD_TRAPPED (traced
                   child has trapped); or CLD_CONTINUED (child continued by SIGCONT).

       waitid(): returns 0 on success or if WNOHANG was specified and no child(ren) spec‐
       ified by id has yet changed state; on error, -1 is returned.
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int
mywaitid(const pid_t pid, siginfo_t* const infop)
{
	if(waitid(P_PID, pid, infop, WEXITED) == 0)
	{
		return pid;
	}
	return -1;
}

static void
pr_exit(const siginfo_t* const infop)
{
	if (infop->si_code & CLD_EXITED)
		printf("normal termination, exit status = %d\n",
		       infop->si_status);
	else if (infop->si_code & CLD_KILLED)
		printf("abnormal termination, signal number = %d\n",
		       infop->si_status);
}

int
main(void)

{
	pid_t pid;
	int status;
	siginfo_t siginfo = {};

	if ((pid = fork()) < 0)
		perror("fork error");
	else if (pid == 0)                  /* child */
		exit(7);

	if (mywaitid(pid, &siginfo) != pid) /* wait for child */
		perror("wait error");
	pr_exit(&siginfo);                  /* and print its status */

	if ((pid = fork()) < 0)
		perror("fork error");
	else if (pid == 0)                  /* child */
		abort();                    /* generates SIGABRT */

	if (mywaitid(pid, &siginfo) != pid) /* wait for child */
		perror("wait error");
	pr_exit(&siginfo);                  /* and print its status */

	if ((pid = fork()) < 0)
		perror("fork error");
	else if (pid == 0)                  /* child */
		status /= 0;                /* divide by 0 generates SIGFPE */

	if (mywaitid(pid, &siginfo) != pid) /* wait for child */
		perror("wait error");
	pr_exit(&siginfo);                  /* and print its status */

	exit(0);
}
