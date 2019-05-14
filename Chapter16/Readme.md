1. Write a program to determine your system’s byte ordering.

   ```c
   #include <stdio.h>
   #include <stdint.h>
   
   int
   main(void)
   {
   	const union {
   		uint64_t number;
   		char bytes[8];
   	} value = {
   		.number = 1
   	};
   
   	printf("%s Endian\n",
   	       (value.bytes[0] == 1) ? "Little" : "Big");
   
   	return 0;
   }
   ```

2. Write a program to print out which `stat` structure members are supported
   for sockets on at least two different platforms, and describe how the results
   differ.

   Here, I interpreted "socket" as "TCP Socket".

   ```c
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
   ```

   On Linux, I get:

   ```
   st_dev:     8
   st_ino:     19338191
   st_nlink:   1
   st_mode:    49663
   st_uid:     1000
   st_gid:     1000
   st_rdev:    0
   st_size:    0
   st_blksize: 4096
   st_blocks:  0
   st_atime:   0
   st_mtime:   0
   st_ctime:   0
   ```

   On MacOS, I get:

   ```
   st_dev:     0
   st_ino:     0
   st_nlink:   0
   st_mode:    49590
   st_uid:     501
   st_gid:     20
   st_rdev:    0
   st_size:    0
   st_blksize: 131072
   st_blocks:  0
   st_atime:   0
   st_mtime:   0
   st_ctime:   0
   ```
   On FreeBSD, I get:

   ```
   st_dev:     0
   st_ino:     0
   st_nlink:   0
   st_mode:    49590
   st_uid:     1000
   st_gid:     1000
   st_rdev:    0
   st_size:    0
   st_blksize: 32768
   st_blocks:  0
   st_atime:   0
   st_mtime:   0
   st_ctime:   0
   ```

   All three reported `st_mode`, `st_uid`, `st_gid`, and `st_blksize`; the
   `st_uid` and `st_gid` fields were consistently the UID and GID of the
   user who ran the program.

   In addition to those fields, Linux also reported `st_dev`, `st_ino`,
   and `st_nlink`.

3. The program in Figure 16.17 provides service on only a single endpoint.
   Modify the program to support service on multiple endpoints (each with a
   different address) at the same time.

   Here's the program (also in `exercise_3.c`):
   ```c
   #include <netdb.h>
   #include <errno.h>
   #include <fcntl.h>
   #include <signal.h>
   #include <stdlib.h>
   #include <stdio.h>
   #include <string.h>
   #include <sys/resource.h>
   #include <sys/socket.h>
   #include <sys/stat.h>
   #include <sys/time.h>
   #include <sys/types.h>
   #include <unistd.h>
   
   #define err_sys(fmt, ...) fprintf(stderr, (fmt "\n"), ##__VA_ARGS__)
   #define err_quit(fmt, ...) do { err_sys(fmt, ##__VA_ARGS__); exit(1); } while(0)
   
   #define syslog(level, fmt, ...) fprintf(stderr, (fmt "\n"), ##__VA_ARGS__)
   
   #define BUFLEN 128
   #define QLEN 10
   
   static int initserver(int, const struct sockaddr*, socklen_t, int);
   static int set_cloexec(int fd);
   
   void
   serv(int sockfd)
   {
   	set_cloexec(sockfd);
   
   	for (;;) {
   		const int clfd = accept(sockfd, NULL, NULL);
   		char buf[BUFLEN];
   
   		if (clfd < 0) {
   			syslog(LOG_ERR, "ruptimed: accept error: %s",
   			       strerror(errno));
   			exit(1);
   		}
   		set_cloexec(clfd);
   
   		FILE* const fp = popen("/usr/bin/uptime", "r");
   		if (fp == NULL) {
   			sprintf(buf, "error: %s\n", strerror(errno));
   			send(clfd, buf, strlen(buf), 0);
   		} else {
   			while (fgets(buf, BUFLEN, fp) != NULL) {
   				send(clfd, buf, strlen(buf), 0);
   			}
   			pclose(fp);
   		}
   		close(clfd);
   	}
   }
   
   int
   main(int argc, char* argv[])
   {
   	if (argc != 1) {
   		err_quit("usage: %s", argv[0]);
   	}
   
   	const struct sockaddr_in server_sock = {
   		.sin_family = AF_INET,
   		.sin_addr.s_addr = htonl(INADDR_ANY),
   		.sin_port = htons(24482),
   	};
   
   	const int sockfd = initserver(SOCK_STREAM,
   	                              (const struct sockaddr*) &server_sock,
   	                              sizeof(server_sock),
   	                              QLEN);
   	if (sockfd >= 0) {
   		serv(sockfd);
   		exit(0);
   	}
   
   	exit(0);
   }
   
   static int
   initserver(int type, const struct sockaddr* addr, socklen_t alen, int qlen)
   {
   	int err = 0;
   
   	const int fd = socket(addr->sa_family, type, 0);
   	if (fd < 0) {
   		return -1;
   	}
   
   	if (bind(fd, addr, alen) < 0) {
   		goto errout;
   	}
   
   	if (type == SOCK_STREAM || type == SOCK_SEQPACKET) {
   		if (listen(fd, qlen) < 0) {
   			goto errout;
   		}
   	}
   	return fd;
   
   errout:
   	err = errno;
   	close(fd);
   	errno = err;
   	return -1;
   }
   
   static int
   set_cloexec(int fd)
   {
   	const int val = fcntl(fd, F_GETFD, 0);
   	if (val < 0) {
   		return -1;
   	}
   
   	/* enable close-on-exec */
   	return fcntl(fd, F_SETFD, val | FD_CLOEXEC);
   }
   ```

   I modified Figure 16.17 in a few ways. First, I elected not to `daemonize`
   the process, not because it ought not be, but for learning/experimenting
   I didn't want it to be a daemon.  Also, I change it from using
   `getaddrinfo` to just setting up the socket myself. This enabled me to
   have a single socket to handle all addresses.  (It also makes it so that
   I don't need to add an entry to `/etc/services` in order to test.)  Doing
   this satisifies the "different address" requirement for the exercise.
   I could have kept the call to `getaddrinfo` and walked the `ailist` and
   `fork`ed or created a thread to handle each.  Since handling a request
   is a short-lived operaiton, I didn't parallelize handling the request.
   If we wanted to, again, I could have `fork`ed or created a thread to handle
   each.

4. Write a client program and a server program to return the number of processes
   currently running on a specified host computer.

   Server:

   ```c
   #include <arpa/inet.h>
   #include <errno.h>
   #include <stdio.h>
   #include <stdlib.h>
   #include <string.h>
   #include <unistd.h>
   
   #define BUFLEN 128
   #define QLEN    10
   
   #define syslog(level, fmt, ...) fprintf(stderr, (fmt "\n"), ##__VA_ARGS__)
   
   static int
   initserver(int type, const struct sockaddr* addr, socklen_t alen, int qlen)
   {
   	int err = 0;
   
   	const int fd = socket(addr->sa_family, type, 0);
   	if (fd < 0) {
   		return -1;
   	}
   
   	if (bind(fd, addr, alen) < 0) {
   		goto errout;
   	}
   
   	if (type == SOCK_STREAM || type == SOCK_SEQPACKET) {
   		if (listen(fd, qlen) < 0) {
   			goto errout;
   		}
   	}
   	return fd;
   
   errout:
   	err = errno;
   	close(fd);
   	errno = err;
   	return -1;
   }
   
   void
   serv(int sockfd)
   {
   	for (;;) {
   		const int clfd = accept(sockfd, NULL, NULL);
   		char buf[BUFLEN];
   
   		if (clfd < 0) {
   			syslog(LOG_ERR, "ruptimed: accept error: %s",
   			       strerror(errno));
   			exit(1);
   		}
   
   		FILE* fp = popen("ls -d /proc/[0-9]* | wc -l", "r");
   		if (fp == NULL) {
   			sprintf(buf, "error: %s\n", strerror(errno));
   			send(clfd, buf, strlen(buf), 0);
   		} else {
   			while (fgets(buf, BUFLEN, fp) != NULL) {
   				send(clfd, buf, strlen(buf), 0);
   			}
   			pclose(fp);
   		}
   		close(clfd);
   	}
   }
   
   int
   main(void)
   {
   	const struct sockaddr_in server_sock = {
   		.sin_family = AF_INET,
   		.sin_addr.s_addr = htonl(INADDR_ANY),
   		.sin_port = htons(24482),
   	};
   
   	const int sockfd = initserver(SOCK_STREAM,
   	                              (const struct sockaddr*) &server_sock,
   	                              sizeof(server_sock),
   	                              QLEN);
   	if (sockfd >= 0) {
   		serv(sockfd);
   		exit(0);
   	}
   
   	return 0;
   }
   ```

   Client:

   ```c
   #include <arpa/inet.h>
   #include <netdb.h>
   #include <stdio.h>
   #include <stdlib.h>
   #include <string.h>
   #include <sys/socket.h>
   #include <unistd.h>
   
   #define BUFLEN 128
   
   static void
   client(const int sockfd)
   {
   	char buffer[BUFLEN] = {};;
   
   	if (recv(sockfd, buffer, sizeof(buffer) - 1, 0) < 0) {
   		perror("read");
   		return;
   	}
   
   	printf("Num procs: %s", buffer);
   }
   
   int
   main(const int argc, const char* const argv[])
   {
   	if (argc < 3) {
   		fprintf(stderr, "Usage: %s <host> <port>\n", argv[0]);
   		return 1;
   	}
   	const char* const host = argv[1];
   	const char* const port_str = argv[2];
   	const int port = strtol(port_str, NULL, 10);
   
   	const int sockfd = socket(AF_INET, SOCK_STREAM, 0);
   	if (sockfd < 0) {
   		perror("socket");
   		return 1;
   	}
   
   	const struct sockaddr_in servaddr = {
   		.sin_family = AF_INET,
   		.sin_addr.s_addr = inet_addr(host),
   		.sin_port = htons(port),
   	};
   
   	if (connect(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
   		perror("connect");
   		close(sockfd);
   		return 1;
   	}
   
   	client(sockfd);
   	close(sockfd);
   
   	return 0;
   }
   ```


5. In the program in Figure 16.18, the server waits for the child to execute
   the `uptime` command and exit before accepting the next connect request.
   Redesign the server so that the time to service one request doesn’t delay
   the processing of incoming connect requests.

   Here's the program (also in `exercise_5.c`):
   ```c
   #include <errno.h>
   #include <fcntl.h>
   #include <netdb.h>
   #include <signal.h>
   #include <stdio.h>
   #include <stdlib.h>
   #include <string.h>
   #include <syslog.h>
   #include <sys/resource.h>
   #include <sys/stat.h>
   #include <sys/socket.h>
   #include <sys/time.h>
   #include <sys/wait.h>
   #include <unistd.h>
   
   #define QLEN 10
   
   #ifndef HOST_NAME_MAX
   #define HOST_NAME_MAX 256
   #endif
   
   #define err_sys(fmt, ...) fprintf(stderr, (fmt "\n"), ##__VA_ARGS__)
   #define err_quit(fmt, ...) do { err_sys(fmt, ##__VA_ARGS__); exit(1); } while(0)
   
   /* Track if we have artifically delayed the first client yet */
   static int done_child_wait = 0;
   
   static int initserver(int, const struct sockaddr *, socklen_t, int);
   static void daemonize(const char *cmd);
   static int set_cloexec(int fd);
   
   void
   serve(int sockfd)
   {
   	int clfd, status;
   	pid_t pid;
   
   	set_cloexec(sockfd);
   
   	for (;;) {
   		if ((clfd = accept(sockfd, NULL, NULL)) < 0) {
   			syslog(LOG_ERR, "ruptimed: accept error: %s",
   					strerror(errno));
   			exit(1);
   		}
   		if ((pid = fork()) < 0) {
   			syslog(LOG_ERR, "ruptimed: fork error: %s",
   					strerror(errno));
   			exit(1);
   		} else if (pid == 0) {  /* child */
   			/*
   			 * The parent called daemonize (Figure 13.1), so
   			 * STDIN_FILENO, STDOUT_FILENO, and STDERR_FILENO
   			 * are already open to /dev/null.  Thus, the call to
   			 * close doesn’t need to be protected by checks that
   			 * clfd isn’t already equal to one of these values.
   			 */
   			if (dup2(clfd, STDOUT_FILENO) != STDOUT_FILENO ||
   					dup2(clfd, STDERR_FILENO) != STDERR_FILENO) {
   				syslog(LOG_ERR, "ruptimed: unexpected error");
   				exit(1);
   			}
   			close(clfd);
   
   			/* Artifically delay the first client */
   			if(done_child_wait == 0) {
   				sleep(10);
   			}
   
   			execl("/usr/bin/uptime", "uptime", (char *)0);
   			syslog(LOG_ERR, "ruptimed: unexpected return from exec: %s",
   					strerror(errno));
   			exit(1);
   		} else { /* parent */
   			/* Don't delay any future clients */
   			done_child_wait = 1;
   			close(clfd);
   		}
   	}
   }
   
   static void
   sigchld_handler(int signo)
   {
   	// Reap any children that have terminated
   	while(waitpid(-1, NULL, WNOHANG) > 0);
   }
   
   int
   main(int argc, char *argv[])
   {
   	struct addrinfo *ailist, *aip;
   	struct addrinfo hint;
   	int             sockfd, err, n;
   	char            *host;
   
   	if (argc != 1)
   		err_quit("usage: ruptimed");
   
   	if (signal(SIGCHLD, sigchld_handler) < 0) {
   		perror("signal");
   		return 1;
   	}
   
   	if ((n = sysconf(_SC_HOST_NAME_MAX)) < 0)
   		n = HOST_NAME_MAX;  /* best guess */
   
   	if ((host = malloc(n)) == NULL)
   		err_sys("malloc error");
   
   	if (gethostname(host, n) < 0)
   		err_sys("gethostname error");
   
   	daemonize("ruptimed");
   
   	memset(&hint, 0, sizeof(hint));
   	hint.ai_flags = AI_CANONNAME;
   	hint.ai_socktype = SOCK_STREAM;
   	hint.ai_canonname = NULL;
   	hint.ai_addr = NULL;
   	hint.ai_next = NULL;
   
   	if ((err = getaddrinfo(host, "ruptime", &hint, &ailist)) != 0) {
   		syslog(LOG_ERR, "ruptimed: getaddrinfo error: %s",
   				gai_strerror(err));
   		exit(1);
   	}
   	for (aip = ailist; aip != NULL; aip = aip->ai_next) {
   		if ((sockfd = initserver(SOCK_STREAM, aip->ai_addr,
   		                         aip->ai_addrlen, QLEN)) >= 0) {
   			serve(sockfd);
   			exit(0);
   		}
   	}
   	exit(1); 
   }
   
   static void
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
   
   static int
   initserver(int type, const struct sockaddr* addr, socklen_t alen, int qlen)
   {
   	int fd;
   	int err = 0;
   
   	if ((fd = socket(addr->sa_family, type, 0)) < 0) {
   		return -1;
   	}
   
   	if (bind(fd, addr, alen) < 0) {
   		goto errout;
   	}
   
   	if (type == SOCK_STREAM || type == SOCK_SEQPACKET) {
   		if (listen(fd, qlen) < 0) {
   			goto errout;
   		}
   	}
   	return fd;
   
   errout:
   	err = errno;
   	close(fd);
   	errno = err;
   	return -1;
   }
   
   static int
   set_cloexec(int fd)
   {
   	int val;
   
   	if ((val = fcntl(fd, F_GETFD, 0)) < 0) {
   		return -1;
   	}
   
   	/* enable close-on-exec */
   	return fcntl(fd, F_SETFD, val | FD_CLOEXEC);
   }
   ```

   The approach was to update the parent process to no longer block waiting
   for the child to terminate.  Instead, the parent registers a signal handler
   for `SIGCHLD`, and loops on a non-blocking call to `waitpid` for any
   child while there are children to reap.  This enables the parent process
   to loop back around and handle new incoming requests.

   To test this, I added an artifical `sleep` to the child process that
   handles the first client with some state to ensure that the delay is
   applied only to the first client.  With that, I can connect one client
   that gets blocked waiting for the `sleep` to complete.  Meanwhile, I can
   connect other clients and those clients get an immediate response.

6. Write two library routines: one to enable asynchronous (signal-based) I/O
   on a socket and one to disable asynchronous I/O on a socket. Use Figure
   16.23 to make sure that the functions work on all platforms with as many
   socket types as possible.

   Here's the two functions and a sample program that exercises them (also
   in `exercise_6.c`):

   ```c
   #include <arpa/inet.h>
   #include <errno.h>
   #include <fcntl.h>
   #include <signal.h>
   #include <stdio.h>
   #include <string.h>
   #include <sys/ioctl.h>
   #include <sys/socket.h>
   #include <sys/types.h>
   #include <unistd.h>
   
   #define QLEN 10
   #define BUFLEN 128
   
   typedef void (*signal_handler_fn)(int);
   
   sig_atomic_t stop = 0;
   
   /**
    * Enable asynchronous I/O on the given sockfd with the given handler for
    * processing SIGIO signals.
    */
   static int
   enable_async_io(const int sockfd, signal_handler_fn handler)
   {
   	/*
   	 * Make read non-blocking.  We could use `recv` with `MSG_DONTWAIT`
   	 * but that's a Linux-only API and the exercise wanted to target as
   	 * many platforms as possible.
   	 */
   	const int flags = fcntl(sockfd, F_GETFL, 0);
   	if (flags < 0) {
   		perror("fcntl");
   		return -1;
   	}
   
   	if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
   		perror("fcntl");
   		return -1;
   	}
   
   	/*
   	 * Register the signal handler before enabling async I/O. If we don't
   	 * do this first, we have a race between enabling FIOASYNC and
   	 * registering the signal handler where we might receive data and
   	 * the default signal handler will terminate the process.
   	 */
   	if (signal(SIGIO, handler) < 0) {
   		perror("signal");
   		return -1;
   	}
   
   	/* Set the owner of the socket to this pid */
   	if (fcntl(sockfd, F_SETOWN, getpid()) < 0) {
   		perror("fcntl");
   		return -1;
   	}
   
   	/* Enable async I/O on the socket */
   	const int enabled = 1;
   	if (ioctl(sockfd, FIOASYNC, &enabled) < 0) {
   		perror("ioctl");
   		return -1;
   	}
   
   	return 0;
   }
   
   /**
    * Disable asynchronous I/O on the given sockfd.
    */
   static int
   disable_async_io(const int sockfd)
   {
   	/* Disable async I/O */
   	const int enabled = 0;
   	if (ioctl(sockfd, FIOASYNC, &enabled) < 0) {
   		perror("ioctl");
   		return -1;
   	}
   
   	/* Reset the owner of the socket */
   	if (fcntl(sockfd, F_SETOWN, 0) < 0) {
   		perror("fcntl");
   		return -1;
   	}
   
   	/* Reset the default signal handler */
   	if (signal(SIGIO, SIG_DFL) < 0) {
   		perror("signal");
   		return -1;
   	}
   
   	/* Clear the non-blocking flag on the file descriptor. */
   	const int flags = fcntl(sockfd, F_GETFL, 0);
   	if (flags < 0) {
   		perror("fcntl");
   		return -1;
   	}
   
   	if (fcntl(sockfd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
   		perror("fcntl");
   		return -1;
   	}
   
   	return 0;
   }
   
   /* The async I/O file descriptor. */
   static int client_socket_fd;
   
   static void
   sigio_handler(int signo)
   {
   	char buffer[BUFLEN] = {};
   	int n;
   
   	while ((n = read(client_socket_fd, buffer, sizeof(buffer) - 1)) > 0) {
   		printf("%s", buffer);
   
   		if (strncmp(buffer, "stop", 4) == 0) {
   			stop = 1;
   		}
   	}
   
   	if ((n == 0) || 
   	    ((n == -1) && (errno != EAGAIN) && (errno != EWOULDBLOCK))) {
   		stop = 1;
   	}
   }
   
   int
   main(void)
   {
   	const int server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
   	if (server_socket_fd < 0) {
   		perror("socket");
   		return 1;
   	}
   
   	/*
   	 * Set the SO_REUSEADDR socket option so that we can restart the
   	 * server without waiting for the timeout.
   	 */
   	const int enable = 1;
   	if (setsockopt(server_socket_fd,
   	               SOL_SOCKET,
   	               SO_REUSEADDR,
   	               &enable,
   	               sizeof(int)) < 0) {
   		perror("setsockopt");
   		return 1;
   	}
   
   	const struct sockaddr_in server = {
   		.sin_family = AF_INET,
   		.sin_port = htons(24482),
   		.sin_addr.s_addr = INADDR_ANY,
   	};
   
   	if (bind(server_socket_fd,
   	         (const struct sockaddr*) &server,
   	         sizeof(server)) < 0) {
   		perror("bind");
   		return 1;
   	}
   
   	if (listen(server_socket_fd, QLEN) < 0) {
   		perror("listen");
   		return 1;
   	}
   
   	struct sockaddr_in client = {};
   	socklen_t sock_len = sizeof(client);
   
   	client_socket_fd = accept(server_socket_fd,
   	                          (struct sockaddr*)&client,
   	                          &sock_len);
   	if (client_socket_fd < 0) {
   		perror("accept");
   		return 1;
   	}
   
   	if (enable_async_io(client_socket_fd, sigio_handler) < 0) {
   		return 1;
   	}
   
   	while (!stop) {
   		sleep(1);
   	}
   
   	if (disable_async_io(client_socket_fd) < 0) {
   		return 1;
   	}
   
   	close(client_socket_fd);
   	close(server_socket_fd);
   
   	return 0;
   }
   ```

   The application starts a TCP server on port 24482 and listens for incoming
   connections.  When a client connect, it puts the associated file descriptor
   in asynchronous I/O mode.  It then handles incoming messages from the
   client asynchronously until it reads a message that begins with "stop" or
   until the client disconnects.  It then disable asynchronously I/O mode
   on the client socket and terminates.
