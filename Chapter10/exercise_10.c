/*
10. Write a program that calls `sleep(60)` in an infinite loop. Every five
    times through the loop (every 5 minutes), fetch the current time of day
    and print the `tm_sec` field. Run the program overnight and explain the
    results. How would a program such as the `cron` daemon, which runs every
    minute on the minute, handle this situation?
*/
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

int
main(void)
{
	int i;
	for (i = 0;; ++i) {
		time_t current_time;

		time(&current_time);
		const struct tm* const now = gmtime(&current_time);

		if (i % 5 == 0) {
			printf("%d\n", now->tm_sec);
			fflush(stdout);
		}

		sleep(60);
	}
}
