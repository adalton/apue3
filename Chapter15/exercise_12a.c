#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>

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

		const int queue_id = msgget(key, IPC_CREAT | 0666);
		/* Create the queue */
		if (queue_id < 0) {
			perror("msgget");
			return 1;
		}
		printf("queue_id: %d\n", queue_id);

		/* Delete the queue */
		if (msgctl(queue_id, IPC_RMID, NULL) < 0) {
			perror("msgctl");
			return 1;
		}
	}
	return 0;
}
