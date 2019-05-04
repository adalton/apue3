#include <stdio.h>

int
main(void)
{
	FILE* const file = popen("does-not-exist", "r");

	if (file == NULL) {
		perror("popen");
		return 1;
	}

	pclose(file);

	return 0;
}
