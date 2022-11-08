//#define _GNU_SOURCE
#include <stdio.h>
#include <sys/utsname.h>

int
main(void)
{
	struct utsname info = {};

	if (uname(&info) < 0) {
		perror("uname");
		return 1;
	}

	printf("%s %s %s %s %s",
	       info.sysname,
	       info.nodename,
	       info.release,
	       info.version,
	       info.machine);
#if defined(_GNU_SOURCE)
	printf(" %s", info.domainname);
#endif
	printf("\n");

	return 0;

}
