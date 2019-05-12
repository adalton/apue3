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
