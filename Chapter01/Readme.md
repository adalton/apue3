1. Verify on your system that the directories dot and dot-dot are not the same,
   except in the root directory.

   Note that `/`, `/.`, and `/..` all have the same inode:

       $ /bin/ls -ldi / /. /..
       2 drwxr-xr-x 21 root root 4096 Mar 30 22:43 /
       2 drwxr-xr-x 21 root root 4096 Mar 30 22:43 /.
       2 drwxr-xr-x 21 root root 4096 Mar 30 22:43 /..

   But that in other directories, .. refers to the parent directory

       $ /bin/ls -ldi /etc/ /etc/. /etc/..
       471 drwxr-xr-x 59 root root 4096 Apr  3 22:12 /etc/
       471 drwxr-xr-x 59 root root 4096 Apr  3 22:12 /etc/.
         2 drwxr-xr-x 21 root root 4096 Mar 30 22:43 /etc/..

2. In the output from the program in Figure 1.6, what happened to the processes
   with process IDs 852 and 853?

   We know that two processes (or threads) started between the two executions
   of `a.out`.  Those processes could still be running or not.  Maybe there's
   another user on the system who ran two commands between the two.  Maybe some
   cron job was triggered between the two.  Maybe some existing process started
   threads.  Maybe the user issued two commands between the two executions of
   `a.out` that are not shown.

3. In Section 1.7, the argument to `perror` is defined with the ISO C attribute
   `const`, whereas the integer argument to `strerror` isn’t defined with this
   attribute. Why?

   The `perror` function takes a pointer to the first character in a string of
   characters.  That pointer can point to a string literal:

       perror("foo");

   In this context, it's important that `perror` *use* the value pointed to by
   the given argument but not *modify* the value.  Without the `const`
   attribute, the function could (try to) modify the value pointed to by the
   given pointer.  The `const` enables the compiler to generate a compile-time
   error if the function did try to modify the value pointed to by the given
   pointer.
   
   The `strerror` function doesn't take a pointer, it takes an `int`, which is
   passed by value so the function receives a copy of the value.  There is no
   way for `strerror` to modify the client's copy of the value.

4. If the calendar time is stored as a signed 32-bit integer, in which year
   will it overflow? How can we extend the overflow point? Are these strategies
   compatible with existing applications?

   A signed 32-bit integer can store values [-2147483648, 2147483647].  By
   explicitly saying "signed integer", I assume that the intention is to use
   only the positive values [0, 2147483647].  (If we want to map the negative
   numbers, to dates as well, why not just use a unsigned 32-bit value?)
      
   Time starts at Januay 1, 1970 at 00:00:00 UTC; that will be time t=0.  The
   clock advances 1 tick/second

         2147483647 ticks         * (1 second  / 1 tick)
       = 2147483647 seconds       * (1 minutes / 60 seconds)
       =   35791394.1166 minutes  * (1 hour    / 60 minutes)
       =     596523.235 hours     * (1 day     / 24 hours)
       =      24855.135 days      * (1 year    / 365.25 days)
       =         68 years

   So, the value would overflow in (1970 + 68) = 2038

   To extend the overflow point, we can consider, an *unsigned* 32-bit value,
   that can store values [0, 4294967296].

         4294967296 seconds       * (1 minute / 60 seconds)
       =   71582788.26666 minutes * (1 hour   / 60 minutes)
       =    1193046.47 hours      * (1 day    / 24 hours)
       =      49710.2696 days     * (1 year   / 365.25 days)
       =        136 years

       1970 + 136 = 2106

   This helps, but just kicks the can down the road.  To further extend the
   overlow point, we can consider a signed 64-bit value, that can store values
   [9223372036854775808, 9223372036854775807].

         9223372036854775807 seconds  * (1 minute / 60 seconds)
       =  153722867280912928 minutes  * (1 hours  / 60 minutes)
       =    2562047788015215.5 hours  * (1 day    / 24 hours)
       =     106751991167300.64 days  * (1 year   / 365.25 days)
       =        292271023045.31 years

       1970 + 292271023045 = 292,271,025,015

   Neither of these approaches are compatible with existing applications (at
   least not without recompiling).  Using unsigned 32-bit values instead of
   signed 32-bit values can cause problems with existing applications that
   do arithmetic on these numbers and expect signed results.  Extending it to
   64-bit requires additional storage allocation by the applications.  (If
   the applications are using the proper type abstractions, then they may just
   need to be recompiled).

5. If the process time is stored as a signed 32-bit integer, and if the system
   counts 100 ticks per second, after how many days will the value overflow?

         2147483647 ticks         * (1 second  / 100 ticks)
       =   21474836.47 seconds    * (1 minutes / 60 seconds)
       =     357913.94 minutes    * (1 hour    / 60 minutes)
       =       5965.23 hours      * (1 day     / 24 hours)
       =        248.55 days
