1. When we remotely log in to a BSD system using either `telnet` or `rlogin`,
   the ownership of the PTY slave and its permissions are set, as we described
   in Section 19.3. How does this happen?

   The login daemons are run as root.  Once a remote user is authenticated,
   the daemon changes the user and group of the process to that of the user
   and invokes the user's shell.  While the process is still running as root
   it can update the ownership and the permissions of the PTY device file.

2. Use the `pty` program to determine the values used by your system to
   initialize a slave PTY’s `termios` structure and `winsize` structure.

   Skipping

3. Recode the `loop` function (Figure 19.12) as a single process using either
   `select` or `poll`.

   Skipping

4. In the child process after `pty_fork` returns, standard input, standard
   output, and standard error are all open for read–write. Can you change
   standard input to be read-only and the other two to be write-only?

   No, there is no function to change the read/write status on an open file
   descriptor.

5. In Figure 19.13, identify which process groups are in the foreground and
   which are in the background, and identify the session leaders.

6. In Figure 19.13, in what order do the processes terminate when we type the
   end-of-file character? Verify this with process accounting, if possible.

7. The `script(1)` program normally adds to the beginning of the output file a
   line with the starting time, and to the end of the output file another line
   with the ending time. Add these features to the simple shell script that we
   showed.

   Original script:
   ```sh
   #!/bin/sh
   pty "${SHELL:-/bin/sh}" | tee typescript
   ```

   Modified script:
   ```sh
   #!/bin/sh
   (
   	echo "Starting on $(date)"
   	pty "${SHELL:-/bin/sh}"
   	echo "Ending on $(date)"
   ) | tee typescript
   ```

8. Explain why the contents of the file `data` are output to the terminal in
   the following example, even though the program `ttyname` (Figure 18.16)
   only generates output and never reads its input.
   ```
   $ cat data                          # a file with two lines
   hello,
   world
   $ pty -i < data ttyname             # -i says ignore eof on stdin
   hello,                              # where did these two lines come from?
   world
   fd 0: /dev/ttys005                  # we expect these three lines from ttyname
   fd 1: /dev/ttys005
   fd 2: /dev/ttys005
   ```
9. Write a program that calls `pty_fork` and have the child `exec` another
   program that you will write. The new program that the child `exec`s must
   catch `SIGTERM` and `SIGWINCH`. When it catches a signal, the program should
   print that it did; for the latter signal, it should also print the terminal's
   window size. Then have the parent process send the `SIGTERM` signal to the
   process group of the PTY slave with the `ioctl` command we described in
   Section 19.7. Read back from the slave to verify that the signal was caught.
   Follow this with the parent setting the window size of the PTY slave, and
   then read back the slave’s output again. Have the parent `exit` and
   determine whether the slave process also terminates; if so, how does it
   terminate?
