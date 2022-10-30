#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

/*
   int dup2(int oldfd, int newfd);

   dup2()
       The  dup2() system call performs the same task as dup(), but instead of
       using the lowest-numbered unused file  descriptor,  it  uses  the  file
       descriptor number specified in newfd.  If the file descriptor newfd was
       previously open, it is silently closed before being reused.

       The steps of closing and reusing the file  descriptor  newfd  are  per‐
       formed  atomically.   This  is  important,  because trying to implement
       equivalent functionality using close(2) and dup() would be  subject  to
       race  conditions,  whereby newfd might be reused between the two steps.
       Such reuse could happen because the main program is  interrupted  by  a
       signal  handler that allocates a file descriptor, or because a parallel
       thread allocates a file descriptor.

       Note the following points:

       *  If oldfd is not a valid file descriptor, then the  call  fails,  and
          newfd is not closed.

       *  If oldfd is a valid file descriptor, and newfd has the same value as
          oldfd, then dup2() does nothing, and returns newfd.

RETURN VALUE
       On success, these system calls return  the  new  file  descriptor.   On
       error, -1 is returned, and errno is set appropriately.

ERRORS
       EBADF  oldfd isn't an open file descriptor.

       EBADF  newfd  is out of the allowed range for file descriptors (see the
              discussion of RLIMIT_NOFILE in getrlimit(2)).

       EBUSY  (Linux only) This may be returned by dup2() or dup3()  during  a
              race condition with open(2) and dup().

       EINTR  The  dup2() or dup3() call was interrupted by a signal; see sig‐
              nal(7).

       EMFILE The per-process limit on the number of open file descriptors has
              been  reached  (see  the  discussion  of  RLIMIT_NOFILE in getr‐
              limit(2)).
*/

typedef struct {
	int* data;
	size_t size;
	size_t capacity;
} Stack;

static Stack*
stack_new(void)
{
	Stack* s = malloc(sizeof(Stack));

	assert(s != NULL);

	s->size = 0;
	s->capacity = 64;
	s->data = malloc(s->capacity * sizeof(int));

	assert(s->data != NULL);

	return s;
}

static int
stack_size(Stack* const s)
{
	return s->size;
}

static void
stack_push(Stack* const s, const int item)
{
	if (s->size == s->capacity) {
		s->capacity *= 2;
		s->data = realloc(s->data, s->capacity * sizeof(int));
		assert(s->data != NULL);
	}
	s->data[s->size++] = item;
}

static int
stack_pop(Stack* const s)
{
	return s->data[s->size--];
}

static void
stack_destroy(Stack* const s)
{
	free(s->data);
	free(s);
}

/**
 * Note: I don't know of a way to satisfy the atomic requirement in userspace,
 *       so this implementation is subject to the race conditions mentioned
 *       in the man page.
 *
 *       Also note that this will be subject to limits on the number of
 *       open file descriptors.
 */
static int
my_dup2(const int oldfd, const int newfd)
{
	if (oldfd < 0 || newfd < 0) {
		errno = EBADF;
		return -1;
	}

	if (oldfd == newfd) {
		return newfd;
	}

	int fd = dup(oldfd);

	if (fd < 0) {
		errno = EBADF;
		return -1;
	}

	// oldfd is valid
	close(fd);

	// Silently ignore EBADF
	if (close(newfd) < 0 && errno != EBADF) {
		// errno will be set by close
		return -1;
	}

	Stack* s = stack_new();

	while ((fd = dup(oldfd)) != newfd) {
		stack_push(s, fd);
	}

	// Now newfd is a dup of oldfd, and the stack contains all the
	// intermediate dups
	while (stack_size(s) > 0) {
		fd = stack_pop(s);
		close(fd);
	}

	stack_destroy(s);
	s = NULL;

	return newfd;
}

/*
Sample output:

$ ./a.out
total 0
lrwx------ 1 user group 64 Apr  8 20:56 0 -> /dev/pts/0
lrwx------ 1 user group 64 Apr  8 20:56 1 -> /dev/pts/0
lrwx------ 1 user group 64 Apr  8 20:56 2 -> /dev/pts/0
lr-x------ 1 user group 64 Apr  8 20:56 3 -> /dev/zero
total 0
lrwx------ 1 user group 64 Apr  8 20:56 0 -> /dev/pts/0
lrwx------ 1 user group 64 Apr  8 20:56 1 -> /dev/pts/0
lrwx------ 1 user group 64 Apr  8 20:56 2 -> /dev/pts/0
lr-x------ 1 user group 64 Apr  8 20:56 3 -> /dev/zero
lrwx------ 1 user group 64 Apr  8 20:56 42 -> /dev/pts/0
total 0
lrwx------ 1 user group 64 Apr  8 20:56 1 -> /dev/pts/0
lrwx------ 1 user group 64 Apr  8 20:56 2 -> /dev/pts/0
lr-x------ 1 user group 64 Apr  8 20:56 3 -> /dev/zero
lr-x------ 1 user group 64 Apr  8 20:56 4 -> /dev/zero
lr-x------ 1 user group 64 Apr  8 20:56 42 -> /dev/zero
*/
 
int
main(void)
{
	const int TARGET_FD = 42;
	char buffer[64] = {};

	snprintf(buffer, sizeof(buffer) - 1, "ls -l /proc/%d/fd", getpid());

	// Open some file
	int fd1 = open("/dev/zero", O_RDONLY);
	if (fd1 < 0) {
		perror("open");
		return 1;
	}

	system(buffer);

	// Create a gap
	int fd2 = dup2(1, TARGET_FD);
	if (fd2 < 0) {
		perror("dup2");
		return 1;
	}

	system(buffer);

	// Try to replace TARGET_FD with /dev/zero
	int fd3 = my_dup2(fd1, TARGET_FD);
	if (fd3 < 0) {
		perror("my_dup2");
		return 1;
	}

	system(buffer);

	return 0;
}
