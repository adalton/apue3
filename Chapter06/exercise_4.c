#include <stdio.h>
#include <time.h>

int
main(void)
{
	time_t max_time = 0;
	time_t mask = 1;
	size_t i;

	for (i = 0; i < ((8 * sizeof(time_t)) - 9); ++i)
	{
		max_time |= (mask << i);
	}

	const char* const str = asctime(gmtime(&max_time));
	if (str == NULL) {
		perror("asctime");
		return 1;
	}

	printf("%s\n", str);

	return 0;
}
