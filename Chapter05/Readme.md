1. Implement `setbuf` using `setvbuf`.

2. Type in the program that copies a file using line-at-a-time I/O (`fgets`
   and `fputs`) from Figure 5.5, but use a `MAXLINE` of 4. What happens if you
   copy lines that exceed this length? Explain what is happening.

3. What does a return value of 0 from `printf` mean?

   The `printf` function returns the number of characters printed and -1 on
   error.  If `printf` returns 0, that means that it printed 0 characters.

4. The following code works correctly on some machines, but not on others.
   What could be the problem?

   ```c
   #include <stdio.h>

   int
   main(void)
   {
       char c;

       while ((c = getchar()) != EOF)
           putchar(c);
   }
   ```

   The `getchar` function returns an `int`, not a `char`.  The `EOF` constant
   is -1.  There two cases to consider when `getchar` returns `EOF`: (1) when
   `char` is signed, and (2) when `char` is unsigned.  When `char` is signed,
   then the comparison will terminate the loop; the signed `char` returned
   by `getchar` will be sign-extened to match the number of bits in `EOF`.
   When `char` is unsigned, then the comparison will not terminate the loop;
   the return value of `getchar` will not be sign-extended, and the computer
   will compare `0xFF` to -1.

5. How would you use the `fsync` function (Section 3.13) with a standard
   I/O stream?

   Use the `fileno` function to get the file descriptor assocaited with
   the stream.

   ```c
   fsync(fileno(stdout));
   fsync(fileno(stderr));
   ```

6. In the programs in Figures 1.7 and 1.10, the prompt that is printed does
   not contain a newline, and we don’t call `fflush`. What causes the prompt
   to be output?

   A call to a FILE stream I/O API (like, in this case, `fgets`) will trigger
   a flush.

7. BSD-based systems provide a function called `funopen` that allows us to
   intercept `read`, `write`, `seek`, and `close` calls on a stream. Use this
   function to implement `fmemopen` for FreeBSD and Mac OS X.
