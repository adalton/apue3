#include <shadow.h>
#include <stdio.h>

int
main(void)
{
	const struct spwd* const entry = getspnam("root");

	if (entry != NULL) {
		printf("user/pass: %s/%s\n", entry->sp_namp, entry->sp_pwdp);
	}

	return 0;
}
