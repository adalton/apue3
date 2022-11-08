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

   A call to `fcntl` on `fd1` with a command of `F_SETFL` will affect both
   file desciptors `fd1` and `fd2` -- it will not affect `fd3`.  The operation
   applies to the file table. Descriptors `fd1` and `fd2` share a single file
   table structure, while `fd3` has a separate file table instance.

   See a test implementation in `exercise_3.c`.

   Sample run:

   ```
   $ ./exercise_3
   F_GETFD: initial:      fd1: 0, fd2: 0, fd3: 0
   F_SETFD: Setting FD_CLOEXEC on fd1
   F_GETFD: after change: fd1: 1, fd2: 0, fd3: 0
   
   F_GETFL: initial:      fd1: 8002, fd2: 8002, fd3: 8000
   F_SETFL: Setting O_APPEND on fd1
   F_GETFL: after change: fd1: 8402, fd2: 8402, fd3: 8000
   ```

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
   dup2(fd, 1);   /* does nothing since fd = 1 */
   dup2(fd, 2);   /* fd(1), 0, and 2 are associated with the same file table entry */
   if (fd > 2)    /* fd(1) is not greater than 2 */
       close(fd); /* Not executed */
   /* Now only 0, 1, and 2 are associated with the file table entry */
   ```
   
   Now assume that `fd` is 3:
   
   ```c
   dup2(fd, 0);   /* fd(3) and 0 are associated with the same file table entry */
   dup2(fd, 1);   /* fd(3), 0, and 1 are associated with the same file table entry */
   dup2(fd, 2);   /* fd(3), 0, 1, and 2 are associated with the same file table entry */
   if (fd > 2)    /* fd(3) is greater than 2 */
       close(fd); /* fd(3) is now closed */
   /* Now only 0, 1, and 2 are associated with the file table entry */
   ```

   In both cases, we're left with only file descriptors 0, 1, and 2 associated
   with the file table entry.

5. The Bourne shell, Bourne-again shell, and Korn shell notation
   `digit1>&digit2` says to redirect descriptor `digit1` to the same file as
   descriptor `digit2`. What is the difference between the two commands shown
   below? (Hint: The shells process their command lines from left to right.)

   ```bash
   ./a.out > outfile 2>&1
   ./a.out 2>&1 > outfile
   ```
   
   The first redirects `stdout` (1) to `outfile`, then redirects `stderr` (2)
   to a dup of 1; `stdout` and `stderr` are associated with a single file
   table entry that references `outfile`.
   
   The second redirects `stderr` (2) to whatever `stdout` is associated
   with (likely, the terminal character device file -- the same file that
   `stderr` would have been associated with by default), then redirects
   `stdout` (1) to `outfile` (which doesn't affect `stderr` -- it's still
   associated with the terminal's character device file.

6. If you open a file for read–write with the append flag, can you still read
   from anywhere in the file using `lseek`? Can you use `lseek` to replace
   existing data in the file? Write a program to verify this.

   _Can you still read from anywhere in the file using `lseek`?_

   Yes.

   _Can you use `lseek` to replace existing data in the file?_

   No.  With only `O_APPEND`, the call to `write()` fails (see the example
   program below).  If I modify the sample program to open the file With
   `O_APPEND | O_RDWR`, then the `write()` is successful; it triggers a seek
   to the end of the file before performing the write.

   _Write a program to verify this._

   ```c
   /* exercise_6.c */
   #include <fcntl.h>
   #include <stdio.h>
   #include <string.h>
   #include <unistd.h>
   #include <sys/stat.h>
   #include <sys/types.h>
   
   #define BUFFER_SIZE 1024
   
   int
   main(const int argc, const char* argv[])
   {
   	int exit_status = 1;
   
   	if (argc < 2) {
   		fprintf(stderr, "usage: %s <filename>\n", argv[0]);
   		return 1;
   	}
   
   	const int fd = open(argv[1], O_APPEND /* | O_RDWR */);
   	if (fd < 0) {
   		perror("open");
   		return 1;
   	}
   	char buffer[BUFFER_SIZE] = {};
   
   	if (lseek(fd, 0, SEEK_SET) < 0) {
   		perror("lseek");
   		goto done;
   	}
   
   	if (read(fd, buffer, sizeof(buffer) - 1) < 0) {
   		perror("read");
   		goto done;
   	}
   	printf("%s\n", buffer);
   
   	if (write(fd, "Five", strlen("Five")) < 0) {
   		perror("write");
   		goto done;
   	}
   
   	exit_status = 0;
   
   done:
   	close(fd);
   
   	return exit_status;
   }
   ```

   Sample run:

   ```
   $ ./exercise_6 exercise_6.input
   Four score and seven years ago our fathers brought fourth on this continent
   a new nation, conceived in liberty and dedicated to the proposition that all
   men are created equal.
    
   write: Bad file descriptor
   ```

