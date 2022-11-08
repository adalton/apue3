#include <stdio.h>
#include <unistd.h>

int
main(const int argc, char* const argv[])
{
	if (argc == 1) {
		execlp(argv[0], argv[0], "myarg1", "MY ARG2", NULL);
		perror("execlp");
		return 1;
	}
	int i;

	for (i = 0; i < argc; ++i) {
		printf("argv[%d] = %s\n", i, argv[i]);
	}

	return 0;
}
