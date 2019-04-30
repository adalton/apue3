1. As we might guess from Figure 13.2, when the `syslog` facility is
   initialized, either by calling `openlog` directly or on the first call to
   `syslog`, the special device file for the UNIX domain datagram socket,
   `/dev/log`, has to be opened. What happens if the user process (the daemon)
   calls `chroot` before calling `openlog`?

   After the call to `chroot`, an open of `/dev/log` will be relative to the
   new root.  It's possible that `/dev` was bind mounted to the chroot root;
   if it was, then the call to open the device will be successful.  If not,
   then the open will fail.

   The program could open the device before calling `chroot`; open file
   descriptors from outside a chroot are still valid after calling `chroot`.

2. Recall the sample `ps` output from Section 13.2. The only user-level daemon
   that isn't a session leader is the `rsyslogd` process. Explain why the
   `syslogd` daemon isn't a session leader.

   According to
   [this Stack Exchange Answer](https://unix.stackexchange.com/a/333513/90691),
   it was a bug in `rsyslogd`.

3. List all the daemons active on your system, and identify the function of
   each one.

   ```
   apache2    -- Web Server
   udevd      -- Userspace Device Daemon
   dhcpcd     -- Renew DHCP leases
   sshd       -- Secure Shell Server
   crond      -- Cron Daemon
   dockerd    -- Docker Daemon
   containerd -- Container Daemon (related to Docker)
   ```

4. Write a program that calls the `daemonize` function in Figure 13.1. After
   calling this function, call `getlogin` (Section 8.15) to see whether the
   process has a login name now that it has become a daemon. Print the results
   to a file.

   Here's the program (also in `exercise_4.c`):

   ```c
   #include <fcntl.h>
   #include <signal.h>
   #include <stdio.h>
   #include <stdlib.h>
   #include <syslog.h>
   #include <sys/resource.h>
   #include <sys/stat.h>
   #include <sys/types.h>
   #include <unistd.h>
   
   
   #define err_quit(fmt, ...) \
   	do { fprintf(stderr, fmt "\n", ##__VA_ARGS__); exit(1); } while (0)
   
   void
   daemonize(const char *cmd)
   {
   	int i, fd0, fd1, fd2;
   	pid_t pid;
   	struct rlimit rl = {};
   	struct sigaction sa = {};
   
   	/*
   	 * Clear file creation mask.
   	 */
   	umask(0);
   
   	/*
   	 * Get maximum number of file descriptors.
   	 */
   	if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
   		err_quit("%s: can’t get file limit", cmd);
   
   	/*
   	 * Become a session leader to lose controlling TTY.
   	 */
   	if ((pid = fork()) < 0)
   		err_quit("%s: can’t fork", cmd);
   	else if (pid != 0) /* parent */
   		exit(0);
   	setsid();
   
   	/*
   	 * Ensure future opens won’t allocate controlling TTYs.
   	 */
   	sa.sa_handler = SIG_IGN;
   	sigemptyset(&sa.sa_mask);
   
   	sa.sa_flags = 0;
   	if (sigaction(SIGHUP, &sa, NULL) < 0)
   		err_quit("%s: can’t ignore SIGHUP", cmd);
   	if ((pid = fork()) < 0)
   		err_quit("%s: can’t fork", cmd);
   	else if (pid != 0) /* parent */
   		exit(0);
   	/*
   	 * Change the current working directory to the root so
   	 * we won’t prevent file systems from being unmounted.
   	 */
   	if (chdir("/") < 0)
   		err_quit("%s: can’t change directory to /", cmd);
   	/*
   	 * Close all open file descriptors.
   	 */
   	if (rl.rlim_max == RLIM_INFINITY)
   		rl.rlim_max = 1024;
   	for (i = 0; i < rl.rlim_max; i++)
   		close(i);
   
   	/*
   	 * Attach file descriptors 0, 1, and 2 to /dev/null.
   	 */
   	fd0 = open("/dev/null", O_RDWR);
   	fd1 = dup(0);
   	fd2 = dup(0);
   
   	/*
   	 * Initialize the log file.
   	 */
   	openlog(cmd, LOG_CONS, LOG_DAEMON);
   	if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
   		syslog(LOG_ERR, "unexpected file descriptors %d %d %d",
   				fd0, fd1, fd2);
   		exit(1);
   	}
   }
   
   int
   main(const int argc, const char* const argv[])
   {
   	daemonize(argv[0]);
   	FILE* const f = fopen("/tmp/ex4.out", "w");
   	if (f != NULL) {
   		fprintf(f, "%s\n", getlogin());
   		fclose(f);
   	}
   	return 0;
   }
   ```

   The generated file `/tmp/ex4.out` does contain the login name.
