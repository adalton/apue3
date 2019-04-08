1. We mentioned in Section 2.8 that some of the primitive system data types are
   defined in more than one header. For example, in FreeBSD 8.0, `size_t` is
   defined in 29 different headers. Because all 29 headers could be included
   in a program and because ISO C does not allow multiple typedefs for the same
   name, how must the headers be written?

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
   | -------------- | --------------------------------------------------------------------------------
   | `comp_t`       | `/usr/include/sys/acct.h:typedef uint16_t comp_t;`
   |                | `/usr/include/bits/stdint-uintn.h:typedef __uint16_t uint16_t;`
   |                | `/usr/include/bits/types.h:typedef unsigned short int __uint16_t;`
   | -------------- | --------------------------------------------------------------------------------
   | `dev_t`        | `/usr/include/sys/types.h:typedef __dev_t dev_t;`
   |                | `/usr/include/bits/types.h:__STD_TYPE __DEV_T_TYPE __dev_t;`
   |                | `/usr/include/bits/typesizes.h:#define __DEV_T_TYPE		__UQUAD_TYPE`
   |                | `/usr/include/bits/types.h:# define __UQUAD_TYPE		unsigned long int`
   | -------------- | --------------------------------------------------------------------------------
   | `fd_set`       | `/usr/include/sys/select.h:typedef struct { __fd_mask __fds_bits[__FD_SETSIZE / __NFDBITS]; } fd_set;`
   |                | `/usr/include/sys/select.h:typedef long int __fd_mask;`

3. Update the program in Figure 2.17 to avoid the needless processing that
   occurs when sysconf returns `LONG_MAX` as the limit for `OPEN_MAX`.

   ```c
#include "apue.h"
#include <errno.h>
#include <limits.h>

#ifdef  OPEN_MAX
static long openmax = OPEN_MAX;
#else
static long openmax = 0;
#endif

/*
 * If OPEN_MAX is indeterminate, this might be inadequate.
 */
#define OPEN_MAX_GUESS  256

long
open_max(void)
{
    if (openmax == 0) {     /* first time through */
        errno = 0;
        if ((openmax = sysconf(_SC_OPEN_MAX)) < 0) {
            if (errno == 0)
                openmax = OPEN_MAX_GUESS;   /* it’s indeterminate */
            else
                err_sys("sysconf error for _SC_OPEN_MAX");
        }
    }
    return(openmax);
}```
