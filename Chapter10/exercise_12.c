#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void
sig_handler(const int signo)
{
#define MESSAGE_TEXT "signal received\n"
	write(STDERR_FILENO, MESSAGE_TEXT, sizeof(MESSAGE_TEXT) - 1);
#undef MESSAGE_TEXT
}

#define BUFFER_SIZE (1024 * 1024 * 1024)

int
main(void)
{
	// Allocate a 1GB buffer
	char* const buffer = calloc(sizeof(char), BUFFER_SIZE);
	if (buffer == NULL) {
		perror("calloc");
		return 1;
	}

	FILE* const file = fopen("/tmp/ex12.out", "w");
	if (file == NULL) {
		perror("fopen");
		free(buffer);
		return 1;
	}

	signal(SIGALRM, sig_handler);
	alarm(1);

	const size_t written = fwrite(buffer, BUFFER_SIZE, 1, file);
	if (written != 1) {
		fprintf(stderr, "Failed to write buffer\n");
	}

	fclose(file);
	free(buffer);

	return 0;
}
