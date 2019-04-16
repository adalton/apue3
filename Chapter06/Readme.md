1. If the system uses a shadow file and we need to obtain the encrypted
   password, how do we do so?

   Use `getspnam` (or some other API in `shadown.h`).

2. If you have superuser access and your system uses shadow passwords,
   implement the previous exercise.

   The following program will print the shadow password entry for the root user,
   when run by the superuser.

   ```c
   #include <shadow.h>
   #include <stdio.h>
   
   int main(void)
   {
   	const struct spwd* const entry = getspnam("root");
   
   	if (entry != NULL) {
   		printf("user/pass: %s/%s\n", entry->sp_namp, entry->sp_pwdp);
   	}
   
   	return 0;
   }
   ```

3. Write a program that calls `uname` and prints all the fields in the
   utsname structure. Compare the output to the output from the `uname(1)`
   command.

   Program:

   ```c
    //#define _GNU_SOURCE
    #include <stdio.h>
    #include <sys/utsname.h>
    
    int main(void)
    {
    	struct utsname info = {};
    
    	if (uname(&info) < 0) {
    		perror("uname");
    		return 1;
    	}
    
    	printf("%s %s %s %s %s",
    	       info.sysname,
    	       info.nodename,
    	       info.release,
    	       info.version,
    	       info.machine);
    #if defined(_GNU_SOURCE)
    	printf(" %s", info.domainname);
    #endif
    	printf("\n");
    
    	return 0;
    
    }
   ```

   Output of this program:

   ```
   $ ./a.out
   Linux kosh 4.19.27-gentoo-r1 #1 SMP Sat Mar 9 20:32:43 EST 2019 x86_64
   ```

   Output from `uname -a`:

   ```
   $ uname -a
   Linux kosh 4.19.27-gentoo-r1 #1 SMP Sat Mar 9 20:32:43 EST 2019 x86_64 Intel(R) Xeon(R) CPU E5-2680 v2 @ 2.80GHz GenuineIntel GNU/Linux
   ```

4. Calculate the latest time that can be represented by the `time_t` data type.
   After it wraps around, what happens?

   On my system, `time_t` is defined as:

   ```c
   typedef long int __time_t;
   typedef __time_t time_t;
   ```

   And the `sizeof(time_t)` is 8-bytes (64-bits).  Since `time_t` is signed,
   that leaves us with 63-bits to work with.  2^63 = 9223372036854775807.

         9223372036854775807 seconds  * (1 minute / 60 seconds)
       =  153722867280912928 minutes  * (1 hours  / 60 minutes)
       =    2562047788015215.5 hours  * (1 day    / 24 hours)
       =     106751991167300.64 days  * (1 year   / 365.25 days)
       =        292271023045.31 years

       1970 + 292271023045 = 292,271,025,015

   I tried to write a program (`exercise_4.c`) to test what would happen on
   roll-over, but it failed after using 55 bits.  The maximum date I could
   get `gmtime`/`asctime` to generate was:
 
       $ ./a.out
       Sun Jun 13 06:26:07 1141709097

   Since I couldn't use all 64 bits, I couldn't see what would happen on
   roll-over.

5. Write a program to obtain the current time and print it using `strftime`,
   so that it looks like the default output from `date(1)`. Set the `TZ`
   environment variable to different values and see what happens.

   ```c
   #include <time.h>
   #include <stdio.h>
   
   #define MAX_LINE 256
   
   int main(void)
   {
   	char date_string[MAX_LINE] = {};
   
   	const time_t current_time = time(NULL);
   	if (current_time < 0) {
   		perror("time");
   		return 1;
   	}
   
   	const struct tm* const tm = localtime(&current_time);
   	if (tm == NULL) {
   		perror("localtime");
   		return 1;
   	}
   
   	if (strftime(date_string, sizeof(date_string), "%a %b %d %X %Z %Y\n", tm) == 0) {
   		fprintf(stderr, "strftime failed\n");
   		return 1;
   	}
   
   	printf("%s", date_string);
   
   	return 0;
   } 
   ```

   Sample runs:

   ```
   $ TZ='' ./a.out
   Tue Apr 16 01:54:34 UTC 2019

   $ TZ=Egypt ./a.out
   Tue Apr 16 03:55:08 EET 2019

   $ TZ=EST5EDT ./a.out
   Mon Apr 15 21:55:26 EDT 2019

   $ TZ=US/Hawaii ./a.out
   Mon Apr 15 15:55:56 HST 2019

   $ TZ=US/Alaska ./a.out
   Mon Apr 15 17:56:01 AKDT 2019

   $ TZ=US/Pacific ./a.out
   Mon Apr 15 18:56:11 PDT 2019

   $ TZ=US/Eastern ./a.out
   Mon Apr 15 21:56:16 EDT 2019
   ```
