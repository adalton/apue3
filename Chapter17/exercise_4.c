#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_ARGC_CAPACITY 4  /* Initial max number of arguments in buf */
#define WHITE " \t\n"            /* white space for tokenizing arguments */

/*
 * buf[] contains white-space-separated arguments.  We convert it to an
 * argv-style array of pointers, and call the user’s function (optfunc)
 * to process the array.  We return -1 if there’s a problem parsing buf,
 * else we return whatever optfunc() returns.  Note that user’s buf[]
 * array is modified (nulls placed after each token).
 */
int
buf_args(char* const buf, int (*optfunc)(int, char **))
{
	int retval;
	int argv_capacity = DEFAULT_ARGC_CAPACITY;
	char* ptr;
	int argc;

	char** argv = malloc(argv_capacity * sizeof(char*));
	if (argv == NULL) {
		retval = -1;
		goto out;
	}

	if (strtok(buf, WHITE) == NULL) {   /* an argv[0] is required */
		retval = -1;
		goto out;
	}

	argv[argc = 0] = buf;

	while ((ptr = strtok(NULL, WHITE)) != NULL) {
		if (++argc >= argv_capacity - 1) { /* -1 for room for NULL at end */
			argv_capacity *= 2;
			argv = realloc(argv, argv_capacity * sizeof(char*));
			if (argv == NULL) {
				retval = -1;
				goto out;
			}
		}
		argv[argc] = ptr;
	}
	argv[++argc] = NULL;

	/*
	 * Since argv[] pointers point into the user’s buf[],
	 * user’s function can just copy the pointers, even
	 * though argv[] array will disappear on return.
	 */
	retval = optfunc(argc, argv);

out:
	if (argv != NULL) {
		free(argv);
	}
	return retval;
}

static int
print_args(int argc, char* argv[])
{
	int i;

	for (i = 0; i < argc; ++i) {
		printf("argv[%2d] = %s\n", i, argv[i]);
	}

	return 0;
}

int
main(void)
{
	char buf[] = "These are the voyages of the starship Enterprise."
		     "Its continuing mission, to explore strange new worlds";
	
	printf("Status: %d\n", buf_args(buf, print_args));

	return 0;
}
