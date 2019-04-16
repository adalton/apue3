1. On an Intel x86 system under Linux, if we execute the program that prints
   "hello, world" and do not call `exit` or `return`, the termination status
   of the program --- which we can examine with the shell --- is 13. Why?

   I do not observe this behaior from a C program.  Now a days, if the
   `return` statement is ommitted in `main`, then the compiler implicitly
   adds a `return 0`.

   If we wrote the program in assembly language, then the result would be
   what this question describes. Why? The return value of a function is
   stored in a register.  If that register isn't overwritten, then it will
   still contain the last value stored in it.  The call to `printf` returned
   13 (here, I'm assuming there was also a newline), so that value was left
   in the return value register when `main` terminated.

2. When is the output from the `printf`s in Figure 7.3 actually output?

   Since they include newline characters, if the output is directed to a
   terminal, it happens immediately.  If the output is directed to a file,
   then then it happens when the `stdout` stream is flushed before program
   termination.

3. Is there any way for a function that is called by `main` to examine the
   command-line arguments without (a) passing `argc` and `argv` as arguments
   from `main` to the function or (b) having `main` copy `argc` and `argv`
   into global variables?

   In Linux, the program could read and parse `/proc/self/cmdline`, which
   contains a `NUL` delimiated list of the `argv` values, terminated with
   and additional `NUL` character.

   ```
   "some_command\0arg1\0arg2\0arg3\0\0"
   ```

4. Some UNIX system implementations purposely arrange that, when a program is
   executed, location 0 in the data segment is not accessible. Why?

   If location 0 is accessible, when a pointer contains `NULL`, which is
   almost always 0, then the program could dereference a `NULL` pointer and
   get a valid value.

5. Use the `typedef` facility of C to define a new data type `Exitfunc` for
   an exit handler. Redo the prototype for `atexit` using this data type.

   ```c
   typedef void (*Exitfunc)(void);

   int atexit(Exitfunc function);

   ```

6. If we allocate an array of `long`s using `calloc`, is the array initialized
   to 0? If we allocate an array of pointers using `calloc`, is the array
   initialized to `null` pointers?

   _If we allocate an array of `long`s using `calloc`, is the array initialized
   to 0?_

   Yes, `calloc` 0-fills the allocated memory.
   
   _If we allocate an array of pointers using `calloc`, is the array initialized
   to `null` pointers?_

   Yes (for systems where `NULL` = `(void*) 0`). `calloc` 0-fills the allocated
   memory.

7. In the output from the `size` command at the end of Section 7.6, why aren’t
   any sizes given for the heap and the stack?

8. In Section 7.7, the two file sizes (879443 and 8378) don't equal the sums
   of their respective text and data sizes. Why?

9. In Section 7.7, why does the size of the executable file differ so
   dramatically when we use shared libraries for such a trivial program?

10. At the end of Section 7.10, we showed how a function can’t return a pointer
    to an automatic variable. Is the following code correct?

    ```c
    int
    f1(int val)
    {
        int num = 0;
        int *ptr = &num;

        if (val == 0) {
            int val;

            val = 5;
            ptr = &val;
        }
        return(*ptr + 1);
    }
    ```
