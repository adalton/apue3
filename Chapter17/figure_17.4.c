#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/msg.h>

#define err_sys(fmt, ...) fprintf(stderr, (fmt "\n"), ##__VA_ARGS__)

#define MAXMSZ 512

struct mymesg {
	long mtype;
	char mtext[MAXMSZ];
};

int
main(int argc, char* argv[])
{
	key_t key;
	long qid;
	size_t nbytes;
	struct mymesg m;

	if (argc != 3) {
		fprintf(stderr, "usage: %s <key> <message>\n", argv[0]);
		exit(1);
	}

	key = strtol(argv[1], NULL, 0);
	if ((qid = msgget(key, 0)) < 0) 
		err_sys("Can't open queue key %s", argv[1]);

	memset(&m, 0, sizeof(m));
	strncpy(m.mtext, argv[2], MAXMSZ - 1);
	nbytes = strlen(m.mtext);
	m.mtype = 1;

	if (msgsnd(qid, &m, nbytes, 0) < 0)
		err_sys("can't send message");

	exit(0);
}
