1. We mentioned in Section 2.8 that some of the primitive system data types are
   defined in more than one header. For example, in FreeBSD 8.0, `size_t` is
   defined in 29 different headers. Because all 29 headers could be included
   in a program and because ISO C does not allow multiple typedefs for the same
   name, how must the headers be written?

   Linux, with gcc-8.2.0 has `/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.0/include/stddef.h`.
   It uses the C preprocessor to determine whether or not the `size_t` type
   has been defined, and if not, it defines it.  I'd guess that those macro
   guards are well-known, and that any other files that might end up
   defining it would check (and potentially set) them to prevent other files
   from generating multiple definitions.

2. Examine your system’s headers and list the actual data types used to
   implement the primitive system data types.

   For this, I'll limit the focus to a small subset of the primitive system data
   types listed in Figure 2.21.

   | Type           | Resolved Type
   | -------------- | --------------------------------------------------------------------------------
   | `clock_t`      | `/usr/include/bits/types/clock_t.h:typedef __clock_t clock_t;`
   |                | `/usr/include/bits/types.h:__STD_TYPE __CLOCK_T_TYPE __clock_t;`
   |                | `/usr/include/bits/typesizes.h:#define __CLOCK_T_TYPE		__SYSCALL_SLONG_TYPE`
   |                | `/usr/include/bits/typesizes.h:# define __SYSCALL_SLONG_TYPE	__SLONGWORD_TYPE`
   |                | `/usr/include/bits/types.h:#define __SLONGWORD_TYPE	long int`
   |                |
   | `comp_t`       | `/usr/include/sys/acct.h:typedef uint16_t comp_t;`
   |                | `/usr/include/bits/stdint-uintn.h:typedef __uint16_t uint16_t;`
   |                | `/usr/include/bits/types.h:typedef unsigned short int __uint16_t;`
   |                |
   | `dev_t`        | `/usr/include/sys/types.h:typedef __dev_t dev_t;`
   |                | `/usr/include/bits/types.h:__STD_TYPE __DEV_T_TYPE __dev_t;`
   |                | `/usr/include/bits/typesizes.h:#define __DEV_T_TYPE		__UQUAD_TYPE`
   |                | `/usr/include/bits/types.h:# define __UQUAD_TYPE		unsigned long int`
   |                |
   | `fd_set`       | `/usr/include/sys/select.h:typedef struct { __fd_mask __fds_bits[__FD_SETSIZE / __NFDBITS]; } fd_set;`
   |                | `/usr/include/sys/select.h:typedef long int __fd_mask;`

3. Update the program in Figure 2.17 to avoid the needless processing that
   occurs when sysconf returns `LONG_MAX` as the limit for `OPEN_MAX`.

   This modified version falls back to using the `getrlimit` system call to get
   the `RLIMIT_NOFILE` resource limit if `sysconf` gets an indeterminate value.

   ```c
   #include "apue.h"
   #include <errno.h>
   #include <limits.h>
   #include <unistd.h>
   #include <sys/resource.h>
   
   #ifdef  OPEN_MAX
   static const long DEFAULT_OPEN_MAX = OPEN_MAX;
   #else
   static const long DEFAULT_OPEN_MAX = 0;
   #endif
   
   long
   open_max(void)
   {
       long openmax = DEFAULT_OPEN_MAX;
   
       if (openmax == 0) {
           errno = 0;
           if ((openmax = sysconf(_SC_OPEN_MAX)) < 0) {
               if (errno == 0) {
                   struct rlimit rlim = {};
   
                   if (getrlimit(RLIMIT_NOFILE, &rlim) == 0) {
                       openmax = rlim.rlim_max;
                   } else {
                       err_sys("getrlimit error for RLIMIT_NOFILE");
                   }
               } else {
                   err_sys("sysconf error for _SC_OPEN_MAX");
           }
       }
       return(openmax);
   }
   ```

