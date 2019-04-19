1. In Figure 8.3, we said that replacing the call to `_exit` with a call to
   `exit` might cause the standard output to be closed and `printf` to return
   -1.  Modify the program to check whether your implementation behaves this
   way. If it does not, how can you simulate this behavior?

   My implementation does not behave that way; both `_exit(0)` and `exit(0)`
   yield the same behavior.

   We can simulate this behavior by explicitly closing `stdout` before
   calling `_exit` (or `exit`).

   ```c
   fclose(stdout);
   ```

   See `exercise_1.c`; sample run:

   ```
   $ ./a.out
   before vfork
   $
   ```

2. Recall the typical arrangement of memory in Figure 7.6.  Because the stack
   frames corresponding to each function call are usually stored in the stack,
   and because after a `vfork` the child runs in the address space of the
   parent, what happens if the call to `vfork` is from a function other than
   `main` and the child does a return from this function after the `vfork`?
   Write a test program to verify this, and draw a picture of what's happening.

3. Rewrite the program in Figure 8.6 to use `waitid` instead of `wait`.  Instead
   of calling `p4_exit`, determine the equivalent information from the
   `signinfo` structure.

4. When we execute the program in Figure 8.13 one time, as in:

   ```
   $ ./a.out 
   ```

   the output is correct.  But if we execute the program multiple times, one
   right after the other, as in

   ```
   $ ./a.out ; ./a.out ; ./a.out
   output from parent
   ooutput from parent
   ouotuptut from child
   put from parent
   output from child
   utput from child
   ```

   the output is not correct.  What's happening?  How can we correct this?
   Can this problem happen if we let the child write its output first?

5. In the program shown in Figure 8.20, we call `execl`, specifying the
   *pathname* of the interpreter file.  If we called `execlp` instead,
   specifying a *filename* of `testinterp`, and if the directory
   `/home/sar/bin/` was a path prefix, what would be printed as `argv[2]`
   when the program is run?

6. Write a program that creates a zombie, and then call `system` to execute
   the `ps(1)` command to verify that the process is a zombie.

7. We mentioned in Section 8.10 that POSIX.1 requires open directory streams
   to be closed across an `exec`.  Verify this as follows: call `opendir`
   for the root directory, peek at your system's implementation of the `DIR`
   structure, and print the close-on-exec flag.  Then `open` the same
   directory for reading, and print the close-on-exec flag.
