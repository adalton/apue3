1. In the program shown in Figure 15.6, remove the `close` right before the
   `waitpid` at the end of the parent code. Explain what happens.

   When the child reaches the end of the document, it blocks on a call to
   `read` from the pipe. Since the write-end of the pipe isn't closed in the
   parent, the child will block on the `read` forever.

2. In the program in Figure 15.6, remove the `waitpid` at the end of the parent
   code. Explain what happens.

   The parent reads the file and writes the content to the pipe.  Assuming that
   the size of the file is less than the size of pipe buffer, the parent will
   write the entire file to the buffer, then terminates immediately.

   The behavior of the child depends on the program.  If, for instance, I do:

   ```
   PAGER=/bin/cat ./a.out /etc/services
   ```

   The the content of `/etc/services` fits in the pipe buffer, and `/bin/cat`
   reads from the pipe and writes everything to standard output (the terminal's
   character device) before the parent's exit.

   If I instead do:

   ```
   PAGER=/bin/more ./a.out /etc/services
   ```

   Then I see only the first page of output.  I think that what's happening
   is the parent wrote the entire content of the file to the pipe, and the child
   read the first page and printed it.  Then when the parent terminated, it
   triggered an `exit_group`, which resuted in the child getting terminated.  I
   base this on `man 2 exit_group`:

   > NOTES  
   > &nbsp;Since  glibc  2.3,  this is the system call invoked when the  
   > &nbsp;_exit(2) wrapper function is called.

   And that the `exit` function called by `main` will eventually call `_exit`.

   Finally, if I do:

   ```
   PAGER=/usr/bin/less ./a.out /etc/services
   ```

   Then I don't see any of the output, but I do sometimes see the screen
   "flash".  My guess is that `less` (1) grabs and saves the content of the
   terminal display, (2) renders content to standard output, and (3) restores
   the originally saved display content on exit.  I think the "flash" that
   I see is what was written to standard output, followed immediately by the
   on-exit restoration.

3. What happens if the argument to `popen` is a nonexistent command? Write a
   small program to test this.

4. In the program shown in Figure 15.18, remove the signal handler, execute the
   program, and then terminate the child. After entering a line of input, how
   can you tell that the parent was terminated by `SIGPIPE`?

5. In the program in Figure 15.18, use the standard I/O library for reading
   and writing the pipes instead of `read` and `write`.

6. The Rationale for POSIX.1 gives as one of the reasons for adding the
   `waitpid` function that most pre-POSIX.1 systems can't handle the following:

   ```c
   if ((fp = popen("/bin/true", "r")) == NULL)
   	...
   if ((rc = system("sleep 100")) == -1)
   	...
   if (pclose(fp) == -1)
   	...
   ```

   What happens in this code if `waitpid` isn’t available and `wait` is used
   instead?

7. Explain how `select` and `poll` handle an input descriptor that is a pipe,
   when the pipe is closed by the writer. To determine the answer, write two
   small test programs: one using `select` and one using `poll`.

   Redo this exercise, looking at an output descriptor that is a pipe, when the
   read end is closed.

8. What happens if the _cmdstring_ executed by `popen` with a _type_ of `"r"`
   writes to its standard error?

9. Since `popen` invokes a shell to execute its _cmdstring_ argument, what
   happens when `cmdstring` terminates? (Hint: Draw all the processes involved.)

10. POSIX.1 specifically states that opening a FIFO for read–write is undefined.
    Although most UNIX systems allow this, show another method for opening a
    FIFO for both reading and writing, without blocking.

11. Unless a file contains sensitive or confidential data, allowing other users
    to read the file causes no harm. (It is usually considered antisocial,
    however, to go snooping around in other people's files.) But what happens
    if a malicious process reads a message from a message queue that is being
    used by a server and several clients? What information does the malicious
    process need to know to read the message queue?

12. Write a program that does the following. Execute a loop five times: create
    a message queue, print the queue identifier, delete the message queue. Then
    execute the next loop five times: create a message queue with a key of
    `IPC_PRIVATE`, and place a message on the queue. After the program
    terminates, look at the message queues using `ipcs(1)`. Explain what is
    happening with the queue identifiers.

13. Describe how to build a linked list of data objects in a shared memory
    segment. What would you store as the list pointers?

14. Draw a timeline of the program in Figure 15.33 showing the value of the
    variable `i` in both the parent and child, the value of the long integer
    in the shared memory region, and the value returned by the `update`
    function.  Assume that the child runs first after the `fork`.

15. Redo the program in Figure 15.33 using the XSI shared memory functions from
    Section 15.9 instead of the shared memory-mapped region.

16. Redo the program in Figure 15.33 using the XSI semaphore functions from
    Section 15.8 to alternate between the parent and the child.

17. Redo the program in Figure 15.33 using advisory record locking to alternate
    between the parent and the child.

18. Redo the program in Figure 15.33 using the POSIX semaphore functions from
    Section 15.10 to alternate between the parent and the child.
