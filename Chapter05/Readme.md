1. Implement `setbuf` using `setvbuf`.

   According to the man page for `setbuf`, it is equivalent to

   ```c
   setvbuf(stream, buf, buf ? _IOFBF : _IONBF, BUFSIZ);
   ```

2. Type in the program that copies a file using line-at-a-time I/O (`fgets`
   and `fputs`) from Figure 5.5, but use a `MAXLINE` of 4. What happens if you
   copy lines that exceed this length? Explain what is happening.

   To the user, the program exhibits the same behavior.

   The `fgets` function will stop reading if (1) it has filled the buffer,
   (2) it encounters a newline, or (3) it encounters EOF.  When `MAXLINE`
   is 4, it will read a most 3 characters, leaving one for the `NUL` terminator.
   For lines longer than 3 character, it will read/write 3 characters at a
   time until it reads either a new-line or EOF, in which case it may read
   fewer than 3 characters.

   Although the program exhibits the same behavior, it does perform more
   function calls than the version with a larger buffer.

3. What does a return value of 0 from `printf` mean?

   The `printf` function returns the number of characters printed and -1 on
   error.  If `printf` returns 0, that means that it printed 0 characters.

   ```c
   printf("");
   ```

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
   will compare 256 to -1.

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

   ```c
   // Note: I tested this on MacOS X (Darwin 18.5.0).
   //       I didn't spend a lot of time handling the error cases; I was more
   //       interested in the happy path functionality.
   #include <stdio.h>
   #include <stdlib.h>
   #include <errno.h>
   
   typedef struct {
   	char* buffer;
   	size_t size;
   	fpos_t position;
   } mem_cookie;
   
   static int
   mem_read(void* const c, char* const buf, const int size)
   {
   	mem_cookie* const cookie = c;
   	int i;
   
   	for (i = 0; (i < size) && (cookie->position < cookie->size); ++i) {
   		buf[i] = cookie->buffer[cookie->position++];
   	}
   
   	return i;
   }
   
   static int
   mem_write(void* const c, const char* const buf, const int size)
   {
   	mem_cookie* const cookie = c;
   	int i;
   
   	for (i = 0; (i < size) && (cookie->position < cookie->size); ++i) {
   		cookie->buffer[cookie->position++] = buf[i];
   	}
   
   	return i;
   }
   
   static fpos_t
   mem_seek(void* const c, const fpos_t offset, const int whence)
   {
   	mem_cookie* const cookie = c;
   
   	switch(whence) {
   	case SEEK_SET:
   		if ((offset >= 0) && (offset < cookie->size)) {
   			cookie->position = offset;
   		} else {
   			errno = EINVAL;
   			return -1;
   		}
   		break;
   
   	case SEEK_CUR:
   		if (((cookie->position + offset) < cookie->size) &&
   		    ((cookie->position + offset) >= 0)) {
   			cookie->position += offset;
   		} else {
   			errno = EINVAL;
   			return -1;
   		}
   		break;
   
   	case SEEK_END:
   		if (((cookie->size + offset) < cookie->size) &&
   		    ((cookie->size + offset) >= 0)) {
   			cookie->position = cookie->size + offset;
   		} else {
   			errno = EINVAL;
   			return -1;
   		}
   		break;
   
   	default:
   		errno = EINVAL;
   		return -1;
   	}
   
   	return 0;
   }
   
   static int
   mem_close(void* const c)
   {
   	mem_cookie* const cookie = c;
   
   	free(cookie);
   
   	return 0;
   }
   
   FILE*
   my_fmemopen(void* const buf, const size_t size, const char* const mode)
   {
   	mem_cookie* const cookie = malloc(sizeof(mem_cookie));
   
   	if(cookie == NULL) {
   		perror("malloc");
   		return NULL;
   	}
   
   	cookie->buffer = buf;
   	cookie->size = size;
   	cookie->position = 0;
   
   	// For this example, I'm going to ignore 'mode' and just implement
   	// something that will allow reads and writes.  If I wanted to
   	// implement the mode, I would decode and validate it here and set
   	// some state in the cookie to reflect what is allowed.  Then I'd
   	// check that state in the various I/O functions.
   
   	return funopen(cookie, mem_read, mem_write, mem_seek, mem_close);
   }
   
   int
   main(void)
   {
   	char buffer[256] = {};
   	char buf[6] = {};
   
   	FILE* const f = my_fmemopen(buffer, sizeof(buffer), "w+");
   
   	fprintf(f, "hello, world");
   
   	fseek(f, 0, SEEK_SET);
   	fgets(buf, sizeof(buf), f);
   	printf("read: '%s'\n", buf);
   
   	fseek(f, 0, SEEK_SET);
   	fprintf(f, "12345");
   
   	fclose(f);
   
   	printf("--%s--\n", buffer);
   
   	return 0;
   }
   ```
