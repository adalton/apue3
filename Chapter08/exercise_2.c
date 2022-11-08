#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define LOG(fmt, ...) printf("[%d] %-4s: " fmt "\n", getpid(), __FUNCTION__, ##__VA_ARGS__)

static int
foo()
{
	int x = 42;
	LOG("Entry, before vfork()");
	pid_t pid = vfork();
	LOG("after vfork(), x = %d", x);

	if (pid < 0) {
		perror("vfork");
		exit(1);
	} else if (pid == 0) {
		LOG("return 1");
		return 1;
	}
	LOG("return 0");
	return 0;
}

static int
baz()
{
	int y = 69;
	LOG("Entry");
	pid_t pid = 0;
	LOG("_exit 0");
	_exit(0);
	LOG("return 0");
	return 0;
}

static void
bar()
{
	LOG("Before foo()");
	int ret = foo();
	LOG("After foo()");

	if(ret) {
		LOG("Before baz()");
		baz();
		LOG("After baz()");
	}
}

int
main(void)
{
	LOG("Before bar()");
	bar();
	LOG("After bar()");
	return 0;
}
