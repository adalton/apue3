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
