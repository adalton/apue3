#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>

typedef struct {
	long mtype;
	char mtext[32];
} message_t;

int
main(void)
{
	const int NUM_ITERATIONS = 5;
	int i;

	for (i = 0; i < NUM_ITERATIONS; ++i) {
		/* picked /etc/passwd because it exists */
		const key_t key = ftok("/etc/passwd", i);
		if (key < 0) {
			perror("ftok");
			return 1;
		}

		const int queue_id = msgget(key,
		                            IPC_CREAT | IPC_PRIVATE | 0666);
		/* Create the queue */
		if (queue_id < 0) {
			perror("msgget");
			return 1;
		}

		const message_t msg = {
			.mtype = 1,
			.mtext = "Hello, world!",
		};

		if (msgsnd(queue_id, &msg, sizeof(msg.mtext), 0) < 0) {
			perror("msgsnd");
			return 1;
		}
	}
	return 0;
}
