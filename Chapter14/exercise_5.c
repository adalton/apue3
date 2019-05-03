#include <stdio.h>
#include <sys/select.h>

suseconds_t
sleep_us(const suseconds_t microseconds)
{
	struct timeval timeout = { .tv_usec = microseconds };

	(void) select(0, NULL, NULL, NULL, &timeout);

	return timeout.tv_usec;
}

static suseconds_t
seconds_to_microseconds(double seconds)
{
	return seconds * 1000000U;
}

int
main(void)
{
	sleep_us(seconds_to_microseconds(5.25));
	return 0;
}
