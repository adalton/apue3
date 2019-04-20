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
   within the same address space.  The child returns from `vfork`, then
   returns from `foo` (b).
   
   Next, `foo` calls `baz`. I wrote `baz` so that the size of its stack frame
   would match that of `foo`; the activation record of `baz` directly overlays
   the area of the stack that was used previously by `foo`.  The variable `y`
   in `baz` is at the same locaiton on the stack as `x` was in the previous
   activation of `foo` (c).  `baz` then calls `_exit`, terminating the child.

   At that point, the parent continues execution within the call to `vfork`.
   `vfork` returns to the instruction within `foo` immediately after the
   call to `vfork` (more on why that address didn't get lost when the child
   called other functions below).  Because the child's `baz` stack frame
   overwrote `foo`'s, the value of `x` was overwritten with `baz`'s `y`.

   Although not included in the figure, `foo` and `baz` also record the
   caller's return address at the same location in their stack frames, so the
   child's call to `baz` overwrote the return address stored in the stack
   frame of `foo`.  When the parent returns from `foo`, control jumps to
   the instruction in `bar` after the call to `baz` (even though the parent
   never called `baz`).  `bar` returns to `main`, and `main` returns.

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

   One question that I alluded to aboveis, "How is the return address stored
   on the stack by `vfork` not corrupted by calls to other functions within
   the child?" This problem is not directly assocaited with the abuse of
   `vfork` in this context; this would be a problem even if the child directed
   called `_exit()` or some function in the `exec` family. 

   I took a look at the implementation of the `vfork` function in glibc. I
   noticed that its implementation explicitly copies the return address from
   the stack into a register before invoking the system call.  After the
   system call returns, the implementation rewrites the return address to the
   stack before returning:

   ```
   ENTRY (__vfork)
        /* Pop the return PC value into RDI.  We need a register that
           is preserved by the syscall and that we're allowed to destroy. */
        popq        %rdi
        ...
        /* Stuff the syscall number in RAX and enter into the kernel.  */
        movl        $SYS_ify (vfork), %eax
        syscall
        ...
        /* Push back the return PC.  */
        pushq %rdi
        ...
   ```

   The kernel saves the register state of the parent, and restores it once the
   child terminates, so the return address is not lost.

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
