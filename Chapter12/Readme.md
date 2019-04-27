1. Run the program in Figure 12.17 on a Linux system, but redirect the output
   into a file. Explain the results.

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
