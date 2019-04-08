1. When reading or writing a disk file, are the functions described in this
   chapter really unbuffered? Explain.

   When we say *unbuffered* here, we mean "unbuffered in userspace".  The
   data read/written here likely go through the kernel's buffer cache
   When writing to a block device, the kernel will write an entire block.
   A single `write()` might not write an entire block, and subsequent calls
   to `write()` might need to write to the same block.  Rather than read the
   block, update a part of it, and write it back for every call to `write`,
   the kernel can cache the block in memory and flush (`sync`) it back to
   disk later.

2. Write your own `dup2` function that behaves the same way as the `dup2`
   function described in Section 3.12, without calling the `fcntl` function.
   Be sure to handle errors correctly.

3. Assume that a process executes the following three function calls:
```c
fd1 = open(path, oflags);
fd2 = dup(fd1);
fd3 = open(path, oflags);
```
   Draw the resulting picture, similar to Figure 3.9. Which descriptors are
   affected by an `fcntl` on `fd1` with a command of `F_SETFD`? Which
   descriptors are affected by an `fcntl` on `fd1` with a command of `F_SETFL`?

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
