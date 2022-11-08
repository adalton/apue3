#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

typedef struct {
    const int         fd1;
    const int         fd2;
    const int         fd3;
    const char* const getCmdName;
    const int         getCmd;
    const char* const setCmdName;
    const int         setCmd;
    const char* const setValueName;
    const int         setValue;
} TestArgs;

/**
 * Runs the given getCmd against the given three file descriptors and
 * prints the resulting values.
 */
static void
printValues(const char* const label, const TestArgs* const args)
{
	const int result1 = fcntl(args->fd1, args->getCmd, 0);
	if (result1 < 0) {
	    perror(args->getCmdName);
	}

	const int result2 = fcntl(args->fd2, args->getCmd, 0);
	if (result2 < 0) {
	    perror(args->getCmdName);
	}

	const int result3 = fcntl(args->fd3, args->getCmd, 0);
	if (result3 < 0) {
	    perror(args->getCmdName);
	}

	printf("%s: %s fd1: %x, fd2: %x, fd3: %x\n",
	       args->getCmdName, label, result1, result2, result3);
}

/**
 * Runs the given getCmd and prints the values, then runs the given setCmd
 * against fd1, then runs the given getCmd again and prints the (potentially)
 * updated values.
 */
static void
testFlags(const TestArgs* const args)
{
	printValues("initial:     ", args);

    printf("%s: Setting %s on fd1\n", args->setCmdName, args->setValueName);
	if (fcntl(args->fd1, args->setCmd, args->setValue) < 0) {
	    perror(args->setCmdName);
	}

	printValues("after change:", args);
}

int
main(void)
{
	const char* const path = "/tmp/exercise3.3_testfile";

	const int fd1 = open(path, O_CREAT | O_RDWR, 0644);
	if (fd1 < 0) {
	    perror("open: fd1");
	    return 1;
	}

	const int fd2 = dup(fd1);
	if (fd2 < 0) {
	    perror("dup");
	    return 1;
	}

	const int fd3 = open(path, O_RDONLY);
	if (fd3 < 0) {
	    perror("open: fd3");
	    return 1;
	}


    // Simplify the creation of a TestArgs struct
#   define NewTestArgs(getCmd, setCmd, setValue) \
    { fd1, fd2, fd3, #getCmd, getCmd, #setCmd, setCmd, #setValue, setValue }

    const TestArgs fdTestArgs = NewTestArgs(F_GETFD, F_SETFD, FD_CLOEXEC);
	testFlags(&fdTestArgs);

	printf("\n");

	const TestArgs flTestArgs = NewTestArgs(F_GETFL, F_SETFL, O_APPEND);
	testFlags(&flTestArgs);
	
	return 0;
}
