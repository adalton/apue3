#include <stdio.h>
#include <stdint.h>

int
main(void)
{
	const union {
		uint64_t number;
		char bytes[8];
	} value = {
		.number = 1
	};

	printf("%s Endian\n",
	       (value.bytes[0] == 1) ? "Little" : "Big");

	return 0;
}
