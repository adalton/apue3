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

6. Write two library routines: one to enable asynchronous (signal-based) I/O
   on a socket and one to disable asynchronous I/O on a socket. Use Figure
   16.23 to make sure that the functions work on all platforms with as many
   socket types as possible.
