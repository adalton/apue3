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

   For example:

   ```
   $ hexdump -C /proc/self/cmdline
   00000000  68 65 78 64 75 6d 70 00  2d 43 00 2f 70 72 6f 63  |hexdump.-C./proc|
   00000010  2f 73 65 6c 66 2f 63 6d  64 6c 69 6e 65 00        |/self/cmdline.|
   0000001e
   ```

4. Some UNIX system implementations purposely arrange that, when a program is
   executed, location 0 in the data segment is not accessible. Why?

   If location 0 is accessible, when a pointer contains `NULL`, which is
   almost always 0, then the program could dereference a `NULL` pointer and
   get a "valid" value.  Making location 0 inaccessible ensure that if a
   `NULL` pointer is dereferenced, the program will terminate.

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
   memory.  If you want to be pedantic and protect yourself against a
   hypothetical machine where `NULL` isn't 0, you could explicitly loop over
   the array and write `NULL` to each element.

7. In the output from the `size` command at the end of Section 7.6, why aren’t
   any sizes given for the heap and the stack?

   The `size` command examines the binary; the heap and stack are runtime
   constructs that vary as the program executes.

8. In Section 7.7, the two file sizes (879443 and 8378) don't equal the sums
   of their respective text and data sizes. Why?

   There are segments in the binary other than the text and data segments,
   and the content of those segments take up space in the binary.  There's
   also ELF metadata in the file.  It's also possible that there's some
   alignment requirements for the different sections which leave gaps in the
   binary.  Consider, for example:

   ```
   $ gcc -static hello.c

   $ ls -l a.out
   -rwxr-xr-x 1 user group 957472 Apr 16 19:04 a.out

   $ size ./a.out
      text    data     bss     dec     hex filename
    669371   20940    6016  696327   aa007 ./a.out

   $ size -A ./a.out
   ./a.out  :
   section                 size      addr
   .note.ABI-tag             32   4194704
   .rela.plt                552   4194736
   .init                     23   4195288
   .plt                     184   4195312
   .text                 518944   4195504
   __libc_freeres_fn       3500   4714448
   .fini                      9   4717948
   .rodata               103196   4717984
   .eh_frame              42740   4821184
   .gcc_except_table        191   4863924
   .tdata                    32   6963424
   .tbss                     64   6963456
   .init_array               16   6963456
   .fini_array               16   6963472
   .data.rel.ro           11764   6963488
   .got                     224   6975256
   .got.plt                 208   6975488
   .data                   6896   6975712
   __libc_subfreeres         72   6982608
   __libc_IO_vtables       1704   6982688
   __libc_atexit              8   6984392
   .bss                    5912   6984416
   __libc_freeres_ptrs       40   6990328
   .comment                  34         0
   .debug_aranges           336         0
   .debug_info            53555         0
   .debug_abbrev           6073         0
   .debug_line            23561         0
   .debug_str             15312         0
   .debug_loc             75076         0
   .debug_ranges          10032         0
   Total                 880306
   ```

   The total in the `size -A` output is closer to the actual size than is
   the sum of the text and data segments.

9. In Section 7.7, why does the size of the executable file differ so
   dramatically when we use shared libraries for such a trivial program?

   When compiling the program statically, all of the needed content of the
   libraries on which the program depends are compiled into the executable
   (as opposed to loading that content from a shared library file at runtime).

10. At the end of Section 7.10, we showed how a function can’t return a pointer
    to an automatic variable. Is the following code correct?

    ```c
    int
    f1(int val)
    {
        int num = 0;
        int *ptr = &num;

        if (num == 0) { /* text had val == 0, but val is not in scope */
            int val;

            val = 5;
            ptr = &val;
        }

        return(*ptr + 1);
    }
    ```

    This code is not correct.  The variable `val` is local to the `if` block.
    The code save a pointer to that variable, and dereferences the pointer
    outside of the `if` block, reading a variable that is no longer in scope.
