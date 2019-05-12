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

	struct sockaddr_in server_sock = {
		.sin_family = AF_INET,
		.sin_addr.s_addr = htonl(INADDR_ANY),
		.sin_port = htons(24482),
	};

	const int sockfd = initserver(SOCK_STREAM,
	                              (struct sockaddr*) &server_sock,
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
