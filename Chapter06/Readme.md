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

4. Calculate the latest time that can be represented by the `time_t` data type.
   After it wraps around, what happens?

5. Write a program to obtain the current time and print it using `strftime`,
   so that it looks like the default output from `date(1)`. Set the `TZ`
   environment variable to different values and see what happens.
