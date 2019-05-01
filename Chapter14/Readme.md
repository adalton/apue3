1. Write a test program that illustrates your system’s behavior when a process
   is blocked while trying to write lock a range of a file and additional
   read-lock requests are made. Is the process requesting a write lock starved
   by the processes read locking the file?

2. Take a look at your system’s headers and examine the implementation of
   `select` and the four `FD_` macros.

   ```c
   /* /usr/include/sys/select.h */
   extern int select (int __nfds, fd_set *__restrict __readfds,
                      fd_set *__restrict __writefds,
                      fd_set *__restrict __exceptfds,
                      struct timeval *__restrict __timeout);

   typedef struct
   {
       /* XPG4.2 requires this member name.  Otherwise avoid the name
          from the global namespace.  */
   #ifdef __USE_XOPEN
       __fd_mask fds_bits[__FD_SETSIZE / __NFDBITS];
   # define __FDS_BITS(set) ((set)->fds_bits)
   #else
       __fd_mask __fds_bits[__FD_SETSIZE / __NFDBITS];
   # define __FDS_BITS(set) ((set)->__fds_bits)
   #endif
   } fd_set;

   /* /usr/include/bits/select.h */
   #define FD_SET(fd, fdsetp)      __FD_SET (fd, fdsetp)
   #define FD_CLR(fd, fdsetp)      __FD_CLR (fd, fdsetp)
   #define FD_ISSET(fd, fdsetp)    __FD_ISSET (fd, fdsetp)
   #define FD_ZERO(fdsetp)         __FD_ZERO (fdsetp)

   # define __FD_ZERO(fdsp) \
     do {                                                                        \
       int __d0, __d1;                                                           \
       __asm__ __volatile__ ("cld; rep; " __FD_ZERO_STOS                         \
                             : "=c" (__d0), "=D" (__d1)                          \
                             : "a" (0), "0" (sizeof (fd_set)                     \
                                             / sizeof (__fd_mask)),              \
                               "1" (&__FDS_BITS (fdsp)[0])                       \
                             : "memory");                                        \
     } while (0)
   #define __FD_SET(d, set) \
     ((void) (__FDS_BITS (set)[__FD_ELT (d)] |= __FD_MASK (d)))
   #define __FD_CLR(d, set) \
     ((void) (__FDS_BITS (set)[__FD_ELT (d)] &= ~__FD_MASK (d)))
   #define __FD_ISSET(d, set) \
     ((__FDS_BITS (set)[__FD_ELT (d)] & __FD_MASK (d)) != 0)
   ```
   
3. The system headers usually have a built-in limit on the maximum number of
   descriptors that the `fd_set` data type can handle. Assume that we need to
   increase this limit to handle up to 2,048 descriptors. How can we do this?

4. Compare the functions provided for signal sets (Section 10.11) and the
   `fd_set` descriptor sets. Also compare the implementation of the two on
   your system.

5. Implement the function `sleep_us`, which is similar to `sleep`, but waits
   for a specified number of microseconds. Use either `select` or `poll`.
   Compare this function to the BSD `usleep` function.

6. Can you implement the functions `TELL_WAIT`, `TELL_PARENT`, `TELL_CHILD`,
   `WAIT_PARENT`, and `WAIT_CHILD` from Figure 10.24 using advisory record
   locking instead of signals? If so, code and test your implementation.

7. Determine the capacity of a pipe using nonblocking writes. Compare this
   value with the value of `PIPE_BUF` from Chapter 2.

8. Rewrite the program in Figure 14.21 to make it a filter: read from the
   standard input and write to the standard output, but use the asynchronous
   I/O interfaces. What must you change to make it work properly? Keep in mind
   that you should get the same results whether the standard output is attached
   to a terminal, a pipe, or a regular file.

9. Recall Figure 14.23. Determine the break-even point on your system where
   using `writev` is faster than copying the data yourself and using a single
   `write`.

10. Run the program in Figure 14.27 to copy a file and determine whether the
    last-access time for the input file is updated.

11. In the program from Figure 14.27, `close` the input file after calling
    `mmap` to verify that closing the descriptor does not invalidate the
    memory-mapped I/O.
