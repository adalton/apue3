1. Write a test program that illustrates your system’s behavior when a process
   is blocked while trying to write lock a range of a file and additional
   read-lock requests are made. Is the process requesting a write lock starved
   by the processes read locking the file?

2. Take a look at your system’s headers and examine the implementation of
   `select` and the four `FD_` macros.

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
