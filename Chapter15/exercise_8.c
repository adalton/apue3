#include <stdio.h>

int
main(const int argc, const char* const argv[])
{
	if (argc == 1) {
		char cmd[128];
		snprintf(cmd, sizeof(cmd), "%s foo", argv[0]);
		FILE* proc = popen(cmd, "r");
		if (proc == NULL) {
			perror("fopen");
			return 1;
		}
		pclose(proc);
	} else {
		fprintf(stdout, "Writing to standard output\n");
		fprintf(stderr, "Writing to standard error\n");
	}
	return 0;
}
