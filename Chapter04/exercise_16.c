#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define PATH_ELEMENT "./0123456789"

int
main(const int argc, const char* const argv[])
{
	printf("PATH_MAX: %d\n", PATH_MAX);

	char path[PATH_MAX + 20];
	int len = strlen("/tmp");

	if (chdir("/tmp") < 0) {
		perror("chdir");
		return 1;
	}

	while (len < sizeof(path)) {
		if (mkdir(PATH_ELEMENT, 0755) < 0) {
			perror("mkdir");
			return 1;
		}
		if (chdir(PATH_ELEMENT) < 0) {
			perror("chdir");
			return 1;
		}
		if (getcwd(path, sizeof(path) - 1) < 0) {
			perror("getcwd");
		}

		// -1 to elminate the leading dot
		len += strlen(PATH_ELEMENT) - 1;
	}

	printf("%s\n", path);

	return 0;
}
