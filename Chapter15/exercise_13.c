#include <stdbool.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>

typedef struct {
	int value;
	size_t next;
} list_node_t;

#define NUM_NODES 128

typedef struct {
	ssize_t size;
	ssize_t capacity;
	ssize_t used_list_head;
	ssize_t free_list_head;
	list_node_t list_nodes[NUM_NODES];
} list_t;

void
list_init(list_t* const list)
{
	int i;

	list->size = 0;
	list->capacity = NUM_NODES;
	list->used_list_head = -1;
	list->free_list_head = 0;

	for (i = 0; i < NUM_NODES - 1; ++i) {
		list->list_nodes[i].next = i + 1;
	}
	list->list_nodes[i].next = -1;
}

bool
list_prepend(list_t* const list, const int value)
{
	if (list->size < list->capacity) {
		++list->size;

		ssize_t old_free_list_head = list->free_list_head;
		list->free_list_head = list->list_nodes[old_free_list_head].next;

		list->list_nodes[old_free_list_head].next = list->used_list_head;
		list->list_nodes[old_free_list_head].value = value;
		list->used_list_head = old_free_list_head;

		return true;
	}
	return false;
}

void
list_print(const list_t* const list)
{
	printf("List: ");
	if (list->used_list_head != -1) {
		int next_node = list->used_list_head;

		printf("%d", list->list_nodes[next_node].value);

		next_node = list->list_nodes[next_node].next;
		while (next_node != -1) {
			printf(", %d", list->list_nodes[next_node].value);
			next_node = list->list_nodes[next_node].next;
		}
	}
	printf("\n");
}

int
main(const int argc, const char* const argv[])
{
	const bool create = (argc == 1);
	int flags = 0600;

	/* picked /etc/passwd because it exists; normally I'd use something else */
	const key_t key = ftok("/etc/passwd", 0);
	if (key < 0) {
		perror("ftok");
		return 1;
	}

	if (create) {
		flags |= IPC_CREAT;
	}

	/* Create a shared memory segment */
	const int shm_id = shmget(key, sizeof(list_t), flags);
	if (shm_id < 0) {
		perror("shmget");
		return 1;
	}

	/* Map the shared memory segment into our address space. */
	void* const base = shmat(shm_id, NULL, 0);
	if (base == NULL) {
		perror("shmat");
		return 1;
	}

	list_t* const list = base;

	if (create) {
		printf("Initializing the memory segment...\n");

		/* Initialize the list */
		list_init(list);
	} else {
		/* Add a few elements to the list */
		list_prepend(list, 42);
		list_prepend(list, 17);
		list_prepend(list, -1);
		list_prepend(list, 186);
	}

	list_print(list);

	/* Unmap the shared memory segment from our address space. */
	if (shmdt(base) < 0) {
		perror("shmdt");
		return 1;
	}

	return 0;
}
