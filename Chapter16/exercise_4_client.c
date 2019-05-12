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
