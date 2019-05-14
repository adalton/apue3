1. We chose to use UNIX domain datagram sockets in Figure 17.3, because they
   retain message boundaries. Describe the changes that would be necessary to
   use regular pipes instead. How can we avoid copying the messages two extra
   times?
2. Write the following program using the file descriptor passing functions from
   this chapter and the parent–child synchronization routines from Section 8.9.
   The program calls `fork`, and the child opens an existing file and passes
   the open descriptor to the parent. The child then positions the file using
   `lseek` and notifies the parent. The parent reads the file’s current offset
   and prints it for verification. If the file was passed from the child to the
   parent as we described, they should be sharing the same file table entry, so
   each time the child changes the file’s current offset, that change should
   also affect the parent’s descriptor. Have the child position the file to a
   different offset and notify the parent again.
3. In Figures 17.20 and 17.21, we differentiated between declaring and defining
   the global variables. What is the difference?
4. Recode the `buf_args` function (Figure 17.23), removing the compile-time
   limit on the size of the `argv` array. Use dynamic memory allocation.
5. Describe ways to optimize the function `loop` in Figure 17.29 and
   Figure 17.30.  Implement your optimizations.
6. In the `serv_listen` function (Figure 17.8), we `unlink` the name of the file
   representing the UNIX domain socket if the file already exists. To avoid
   unintentionally removing a file that isn’t a socket, we could call `stat`
   first to verify the file type. Explain the two problems with this approach.
7. Describe two possible ways to pass more than one file descriptor with a
   single call to `sendmsg`. Try them out to see if they are supported by your
   operating system.
