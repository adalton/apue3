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

   The program is in `exercise_2.c` and the image is in `8.2.pdf`.
   In the program `main` calls `bar`, `bar` calls `foo`, and `foo` calls
   `vfork` (a).
   
   The parent blocks at the call to `vfork` and the child continues execution
   within the same address space.  The child then returns from `foo` (b).
   
   Next, `foo` calls baz.  The size of the `foo` and `baz` stack frames are
   the same, so the `baz` stack frame directly overlays the location where
   the `foo` stack frame was placed.  The variable `y` in `baz` is on the same
   locaiton on the stack as `x` was in the call to `foo` (c).  `baz` then
   calls `_exit`, terminating the child.

   At that point, the parent continues execution.  The call to `vfork`
   saved the address of the next instruction within its stack frame, and
   no other function overwrote that location on the stack, so `vfork`
   continued execution of `foo`.  Because the child's `baz` stack frame
   overwrote `foo`, the value of `x` was overwritten with `baz`'s `y`.

   Although not included in the figure, `foo` and `baz` also record the
   address of the next instruction in `bar`.  The execution of `baz` in the
   child overwrote that return address in the same way that it did `x`.
   When the parent then returns from `foo`, it really returns from `baz`
   `bar` then returns to `main`.

   Here's a sample execution of the program:

   ```
   [24583] main: Before bar()
   [24583] bar : Before foo()
   [24583] foo : Entry, before vfork()
   [24584] foo : after vfork(), x = 42 <- child begins; prints x's original value
   [24584] foo : return 1              <- child returns from foo to bar
   [24584] bar : After foo()           <- child continues in bar
   [24584] bar : Before baz()          <- child is about to call baz
   [24584] baz : Entry                 <- child enters baz, creates baz stack frame
   [24584] baz : _exit 0               <- child terminates in baz
   [24583] foo : after vfork(), x = 69 <- parent "returns" from vfork, prints x (overwritten by baz's y)
   [24583] foo : return 0              <- parent returns from foo to bar...
   [24583] bar : After baz()           <- ... after call to baz (child overwrote return address)
   [24583] main: After bar()
   ```

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
