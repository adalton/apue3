#include <time.h>
#include <stdio.h>

#define MAX_LINE 256

int
main(void)
{
	char date_string[MAX_LINE] = {};

	const time_t current_time = time(NULL);
	if (current_time < 0) {
		perror("time");
		return 1;
	}

	const struct tm* const tm = localtime(&current_time);
	if (tm == NULL) {
		perror("localtime");
		return 1;
	}

	if (strftime(date_string, sizeof(date_string), "%a %b %d %X %Z %Y\n", tm) == 0) {
		fprintf(stderr, "strftime failed\n");
		return 1;
	}

	printf("%s", date_string);

	return 0;
} 
