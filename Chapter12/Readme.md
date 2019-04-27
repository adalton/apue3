1. Run the program in Figure 12.17 on a Linux system, but redirect the output
   into a file. Explain the results.

   Running normally:

   ```
   $ ./a.out
   thread started...
   parent about to fork...
   preparing locks...
   parent unlocking locks...
   parent returned from fork
   child unlocking locks...
   child returned from fork
   ```

   Running with output redirected to a file:

   ```
   $ ./a.out > /tmp/out && cat /tmp/out
   thread started...
   parent about to fork...
   preparing locks...
   parent unlocking locks...
   parent returned from fork
   thread started...
   parent about to fork...
   preparing locks...
   child unlocking locks...
   child returned from fork
   ```

   _Explain the results._

   When writing to the terminal device, the output was flushed when a newline
   was encountered.  As a result, every call to `printf` was immediately
   written to the screen.

   When writing to the file, the output was buffered.  At the point where
   the process `fork`ed, the following output was in the buffer; that buffer
   was copied to the child during the `fork`:

   ```
   thread started...
   parent about to fork...
   preparing locks...
   ```

   As the parent and child continued execution, each continued to append to
   their respective buffers.  When the parent `exit`ed, it flushed its buffer,
   writing:

   ```
   thread started...
   parent about to fork...
   preparing locks...
   parent unlocking locks...
   parent returned from fork
   ```
   
   Finally, when the child `exit`ed, it flushed its buffer, writing:

   ```
   thread started...
   parent about to fork...
   preparing locks...
   child unlocking locks...
   child returned from fork
   ```

2. Implement `putenv_r`, a reentrant version of `putenv`. Make sure that your
   implementation is async-signal safe as well as thread-safe.

3. Can you make the `getenv` function shown in Figure 12.13 async-signal safe
   by blocking signals at the beginning of the function and restoring the
   previous signal mask before returning? Explain.

4. Write a program to exercise the version of `getenv` from Figure 12.13.
   Compile and run the program on FreeBSD. What happens? Explain.

5. Given that you can create multiple threads to perform different tasks within
   a program, explain why you might still need to use `fork`.

6. Reimplement the program in Figure 10.29 to make it thread-safe without using
   `nanosleep` or `clock_nanosleep`.

7. After calling `fork`, could we safely reinitialize a condition variable in
   the child process by first destroying the condition variable with
   `pthread_cond_destroy` and then initializing it with `pthread_cond_init`?

8. The `timeout` function in Figure 12.8 can be simplified substantially.
   Explain how.
