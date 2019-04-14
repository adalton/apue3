1. Implement `setbuf` using `setvbuf`.

2. Type in the program that copies a file using line-at-a-time I/O (`fgets`
   and `fputs`) from Figure 5.5, but use a `MAXLINE` of 4. What happens if you
   copy lines that exceed this length? Explain what is happening.

3. What does a return value of 0 from `printf` mean?

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

5. How would you use the `fsync` function (Section 3.13) with a standard
   I/O stream?

6. In the programs in Figures 1.7 and 1.10, the prompt that is printed does
   not contain a newline, and we don’t call `fflush`. What causes the prompt
   to be output?

7. BSD-based systems provide a function called `funopen` that allows us to
   intercept `read`, `write`, `seek`, and `close` calls on a stream. Use this
   function to implement `fmemopen` for FreeBSD and Mac OS X.
