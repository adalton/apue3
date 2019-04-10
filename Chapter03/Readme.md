1. When reading or writing a disk file, are the functions described in this
   chapter really unbuffered? Explain.

   When we say *unbuffered* here, we mean "unbuffered in userspace" (to
   minimize the system call overhead).  The data read/written here will
   usually go through the kernel's buffer cache.
   
   When writing to a block device, the kernel will write an entire block.
   A single `write()` might not write an entire block, and subsequent calls
   to `write()` might need to write to the same block.  Rather than read the
   block, update a part of it, and write it back for every call to `write`,
   the kernel can cache the block in memory and flush (`sync`) it back to
   disk later.  The same idea applies to reading data from blocks.

2. Write your own `dup2` function that behaves the same way as the `dup2`
   function described in Section 3.12, without calling the `fcntl` function.
   Be sure to handle errors correctly.

   Here's my function.  I use a stack to store file descriptors between
   the current maximum and the target newfd.  See the full implementation
   in `my_dup2.c`.

```c
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
	if (newfd < 0 || newfd < 0) {
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
```

3. Assume that a process executes the following three function calls:
```c
fd1 = open(path, oflags);
fd2 = dup(fd1);
fd3 = open(path, oflags);
```
   Draw the resulting picture, similar to Figure 3.9. Which descriptors are
   affected by an `fcntl` on `fd1` with a command of `F_SETFD`? Which
   descriptors are affected by an `fcntl` on `fd1` with a command of `F_SETFL`?

   See the file 3.3.pdf for the resulting picture.

   A call to `fcntl` on `fd1` with a command of `F_SETFD` applies only to
   `fd1`; the operation applies to the file descriptor and each file descriptor
   has its own file descriptor flags.

   A call to `fcntl` on `fd1` with a command of `F_SETFL` will affect all
   three file descriptors; the operation applies to the file and all three
   file descriptors are associated with the same file.

4. The following sequence of code has been observed in various programs:
```c
dup2(fd, 0);
dup2(fd, 1);
dup2(fd, 2);
if (fd > 2)
    close(fd);
```
   To see why the if test is needed, assume that `fd` is 1 and draw a picture
   of what happens to the three descriptor entries and the corresponding file
   table entry with each call to `dup2`. Then assume that `fd` is 3 and draw
   the same picture.

   I'm not drawing the picture for this one, but I will explore the two
   cases.  This code segment is intended to make `stdin`, `stdout`, and
   `stderr` to refer to the same file table entry.

   Assume that `fd` is 1:

```c
dup2(fd, 0);   /* fd(1) and 0 are associated with the same file table entry */
dup2(fd, 1);   /* does nothing since fd = 1*/
dup2(fd, 2);   /* fd(1), 0, and 2 are associated with the same file table entry */
if (fd > 2)    /* fd(1) is not greater than 2 */
    close(fd); /* Not executed */
```

    Now assume that `fd` is 3:

```c
dup2(fd, 0);   /* fd(3) and 0 are associated with the same file table entry */
dup2(fd, 1);   /* fd(3), 0, and 1 are associated with the same file table entry */
dup2(fd, 2);   /* fd(3), 0, 1, and 2 are associated with the same file table entry */
if (fd > 2)    /* fd(3) is greater than 2 */
    close(fd); /* Now only 0, 1, and 2 are associated with the file table entry */
```

5. The Bourne shell, Bourne-again shell, and Korn shell notation
   `digit1>&digit2` says to redirect descriptor `digit1` to the same file as
   descriptor `digit2`. What is the difference between the two commands shown
   below? (Hint: The shells process their command lines from left to right.)
```bash
./a.out > outfile 2>&1
./a.out 2>&1 > outfile
```
6. If you open a file for read–write with the append flag, can you still read
   from anywhere in the file using `lseek`? Can you use `lseek` to replace
   existing data in the file? Write a program to verify this.
