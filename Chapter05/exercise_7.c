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
