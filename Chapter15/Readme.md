1. In the program shown in Figure 15.6, remove the `close` right before the
   `waitpid` at the end of the parent code. Explain what happens.

   When the child reaches the end of the document, it blocks on a call to
   `read` from the pipe. Since the write-end of the pipe isn't closed in the
   parent, the child will block on the `read` forever.

2. In the program in Figure 15.6, remove the `waitpid` at the end of the parent
   code. Explain what happens.

   The parent reads the file and writes the content to the pipe.  Assuming that
   the size of the file is less than the size of pipe buffer, the parent will
   write the entire file to the buffer, then terminates immediately.

   The behavior of the child depends on the program.  If, for instance, I do:

   ```
   PAGER=/bin/cat ./a.out /etc/services
   ```

   The the content of `/etc/services` fits in the pipe buffer, and `/bin/cat`
   reads from the pipe and writes everything to standard output (the terminal's
   character device) before the parent's exit.

   If I instead do:

   ```
   PAGER=/bin/more ./a.out /etc/services
   ```

   Then I see only the first page of output.  I think that what's happening
   is the parent wrote the entire content of the file to the pipe, and the child
   read the first page and printed it.  Then when the parent terminated, it
   triggered an `exit_group`, which resuted in the child getting terminated.  I
   base this on `man 2 exit_group`:


   > NOTES
   > &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Since
   > glibc  2.3,  this is the system call invoked when the _exit(2) wrapper
   > function is called.

   And that the `exit` function called by `main` will eventually call `_exit`.

   Finally, if I do:

   ```
   PAGER=/usr/bin/less ./a.out /etc/services
   ```

   Then I don't see any of the output, but I do sometimes see the screen
   "flash".  My guess is that `less` (1) grabs and saves the content of the
   terminal display, (2) renders content to standard output, and (3) restores
   the originally saved display content on exit.  I think the "flash" that
   I see is what was written to standard output, followed immediately by the
   on-exit restoration.

3. What happens if the argument to `popen` is a nonexistent command? Write a
   small program to test this.

   Here's the program (see also `exercise_3.c`):

   ```c
   #include <stdio.h>

   int
   main(void)
   {
   	FILE* const file = popen("does-not-exist", "r");

   	if (file == NULL) {
   		perror("popen");
   		return 1;
   	}

   	pclose(file);

   	return 0;
   }
   ```

   The output of running this program is:

   ```
   $ ./a.out
   sh: does-not-exist: command not found
   ```

   The call to `popen` successfully starts `/bin/sh`, which tries to invoke
   the non-existant program.  That fails, so `/bin/sh` writes the error
   message to standard error and terminates.

   Since `open` successfully started the shell, it did not fail.

4. In the program shown in Figure 15.18, remove the signal handler, execute the
   program, and then terminate the child. After entering a line of input, how
   can you tell that the parent was terminated by `SIGPIPE`?

   See `exercise_4.c`.

   Running the program I see:

   ```
   $ ./a.out
   1 1
   2
   ```

   Next, I terminate the `add2` process and continue to provide input to the
   parent process.  The parent terminates immediately:

   ```
   2 2
   $
   ```

   I examine the exit status of the process:

   ```
   $ echo $?
   141
   ```

   My shell is `bash`, and according to `man bash`:

   > The return value of a simple command is its exit status, or 128+n if the
   > command is terminated by signal n.

   So `141 - 128 = 13`.  What is signal number 13?

   ```
   $ kill -l 13
   PIPE
   ```

   This shows that the parent process was terminated by a `SIGPIPE` signal.

5. In the program in Figure 15.18, use the standard I/O library for reading
   and writing the pipes instead of `read` and `write`.

   See `exercise_5.c`; I used `fdopen` to create `FILE` streams assocaited
   with the read and write ends of the pipes, and used `fprintf` and `fgets`
   to write to and read from those streams.

6. The Rationale for POSIX.1 gives as one of the reasons for adding the
   `waitpid` function that most pre-POSIX.1 systems can't handle the following:

   ```c
   if ((fp = popen("/bin/true", "r")) == NULL)
   	...
   if ((rc = system("sleep 100")) == -1)
   	...
   if (pclose(fp) == -1)
   	...
   ```

   What happens in this code if `waitpid` isn’t available and `wait` is used
   instead?

   Without `waitpid`, the implementation of `system` would call `wait` to
   wait for the command to terminate.  If the `popen` failed, then the call
   to `system` would return almost immediately --- before the command that it
   executed completes -- and would return the exit status of the `popen`
   command.

   Similarly, the `popen` would also call `wait`, and would block waiting for
   `system`'s child to terminate (if it hadn't already).  It would return the
   exit status of `system`'s child.

7. Explain how `select` and `poll` handle an input descriptor that is a pipe,
   when the pipe is closed by the writer. To determine the answer, write two
   small test programs: one using `select` and one using `poll`.

   Redo this exercise, looking at an output descriptor that is a pipe, when the
   read end is closed.

   1. `select` on read-end
      If a process is blocked on a call to `select` with the read-end of a
      pipe in the `readfds` set, then when the last process that had the
      write-end open closes that file descriptor, `select` will return the
      read-end in `readfds`.

      Sample output of `exercise_7a.c`:

      ```
      $ ./a.out
      fdcount: 1
      readfds set:   1
      writefds set:  0
      exceptfds set: 0
      ```

   2. `select` on write-end
      If a process is blocked on a call to `select` with the write-end of a
      pipe in the `writefds` set, then when the last process that had the
      read-end open closes that file descriptor, `select` will return the
      write-end in `writefds`.

      Sample output of `exercise_7b.c`:

      ```
      $ ./a.out
      fdcount: 1
      readfds set:   0
      writefds set:  1
      exceptfds set: 0
      ```

   3. `poll` on read-end
      If a process is blocked on a call to `poll` with the read-end of a
      pipe in a `struct pollfd` marked with the `POLLIN` event, then when
      the last process that had the write-end closes the file descriptor,
      `poll` will return with `POLLHUP` set in the `revents` field of the
      corresponding `struct pollfd`.

      Sample output of `exercise_7c.c`:

      ```
      $ ./a.out
      fdcount: 1
      revents: POLLHUP
      ```

   4. `poll` on write-end
      If a process calls `poll` with the write-end of a pipe in a
      `struct pollfd` marked with the `POLLOUT` event, if write end of the
      pipe isn't closed and the pipe isn't full (i.e., if data can successfully
      be written to the pipe), then `poll` returns immediately with `POLLOUT`.

      If a process calls `poll` _after_ the write-end of the pipe is closed,
      then `poll` will return immediately with `POLLOUT` and `POLLERR`.

      Sample output of `exercise_7d.c`:

      ```
      $ ./a.out
      fdcount: 1
      revents: POLLOUT POLLERR
      ```

8. What happens if the _cmdstring_ executed by `popen` with a _type_ of `"r"`
   writes to its standard error?

   See `exercise_8.c`.  The `popen` function sets up a pipe and forks to run
   the given command.  With the `"r"` type, it set the pipe up to redirect the
   standard output of the given command to the pipe -- it does not modify
   standard error.  Therefore, the child inherits the standard error file
   descriptor from the calling process.

9. Since `popen` invokes a shell to execute its _cmdstring_ argument, what
   happens when `cmdstring` terminates? (Hint: Draw all the processes involved.)

   The process that calls `popen` forks, execs the shell with the given
   `cmdstring`, and blocks waiting for the shell to terminate.  The shell
   parses the command line arguments, forks, execs the `cmdstring`, and
   blocks waiting for the program to terminate.  When `cmdstring` terminates,
   control returns to the shell.  The shell then exits, returning control
   to the original process.

10. POSIX.1 specifically states that opening a FIFO for read–write is undefined.
    Although most UNIX systems allow this, show another method for opening a
    FIFO for both reading and writing, without blocking.

    One option would be to set up file descriptors that treat it like a
    pipe.  You could:

    * Create the fifo, if it doesn't arleady exist
    * Open the fifo read-only, non-blocking
    * Open the fifo a second time, write-only
    * Get the flags associated with the read-end's file descriptor
    * Update the read-end's file descriptor's flag without `O_NONBLOCK`

    At that point, you have a set of two file descriptors associated with the
    fifo, much like that of a pipe.

    Here's an example (also in `exercise_10.c`):

    ```c
    #include <errno.h>
    #include <fcntl.h>
    #include <stdio.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <sys/wait.h>
    #include <unistd.h>

    static int
    rw_fifo(int fifo_fds[2], const char* const fifo)
    {
    	if (mkfifo(fifo, 0600) < 0) {
    		if (errno != EEXIST) {
    			perror("mkfifo");
    			return -1;
    		}
    	}

    	/* Open read-end non-blocking */
    	fifo_fds[0] = open(fifo, O_RDONLY | O_NONBLOCK);
    	if (fifo_fds[0] < 0) {
    		perror("open");
    		return -1;
    	}

    	/* Open write-end */
    	fifo_fds[1] = open(fifo, O_WRONLY);
    	if (fifo_fds[1] < 0) {
    		perror("open");
    		close(fifo_fds[0]);
    		return -1;
    	}

    	/* Get the flags for the read-end */
    	const int flags = fcntl(fifo_fds[0], F_GETFL);
    	if (flags < 0) {
    		perror("fcntl(F_GETFL)");
    		close(fifo_fds[0]);
    		close(fifo_fds[1]);
    		return -1;
    	}

    	/* Make the read-end blocking */
    	if (fcntl(fifo_fds[0], F_SETFL, flags & ~O_NONBLOCK) < 0) {
    		perror("fcntl(F_SETFL)");
    		close(fifo_fds[0]);
    		close(fifo_fds[1]);
    		return -1;
    	}

    	return 0;
    }

    int
    main(void)
    {
    	const char* const fifo = "/tmp/exercise_15.10.fifo";
    	int fifo_fds[2] = {};

    	if (rw_fifo(fifo_fds, fifo) < 0) {
    		return 1;
    	}

    	const pid_t pid = fork();

    	if (pid < 0) {
    		perror("fork");
    		return 1;
    	}

    	if (pid == 0) {                                        /* child */
    		const char msg[] = "Hello, world!";
    		close(fifo_fds[0]);

    		write(fifo_fds[1], msg, sizeof(msg) - 1);
    	} else {                                               /* parent */
    		char line[128] = {};

    		close(fifo_fds[1]);

    		read(fifo_fds[0], line, sizeof(line) - 1);
    		printf("%s\n", line);

    		wait(NULL);
    	}

    	return 0;
    }
    ```

11. Unless a file contains sensitive or confidential data, allowing other users
    to read the file causes no harm. (It is usually considered antisocial,
    however, to go snooping around in other people's files.) But what happens
    if a malicious process reads a message from a message queue that is being
    used by a server and several clients? What information does the malicious
    process need to know to read the message queue?

    _What happens if a malicious process reads a message from a message queue
    that is being used by a server and several clients?_

    If a malicious process reads a message from a message queue, the message
    is removed from that queue (unless they used the Linux-specific `MSG_COPY`
    message flag).  Reading the message would prevent the server from receiving
    and processing the message.  Depending on what messages are being exchanged
    by the clients and server, the server may or may not be able to detect
    missing messages.

    _What information does the malicious process need to know to read the
    message queue?_

    A malicious process would need the ID of the target message queue.

12. Write a program that does the following. Execute a loop five times: create
    a message queue, print the queue identifier, delete the message queue. Then
    execute the next loop five times: create a message queue with a key of
    `IPC_PRIVATE`, and place a message on the queue. After the program
    terminates, look at the message queues using `ipcs(1)`. Explain what is
    happening with the queue identifiers.

    For the first part of the question (also in `exercise_12a.c`):

    ```c
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
    ```

    Sample output on Linux (4.19.27):

    ```
    $ ./a.out
    queue_id: 917504
    queue_id: 950272
    queue_id: 983040
    queue_id: 1015808
    queue_id: 1048576
    ```

	Note that each queue_id 32768 larger than the previous:

    ```
     917504 + 32768 =  950272
     950272 + 32768 =  983040
     983040 + 32768 = 1015808
    1015808 + 32768 = 1048576
    1048576
    ```

    For the second part of the question (also in `exercise_12b.c`):

    ```c
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
    ```

    Here's the output of `ipcs` after running the above program:

    ```
    $ ipcs -q

    ------ Message Queues --------
    key        msqid      owner      perms      used-bytes   messages
    0x00000cf3 1081344    user       666        32           1
    0x01000cf3 1114113    user       666        32           1
    0x02000cf3 1146882    user       666        32           1
    0x03000cf3 1179651    user       666        32           1
    0x04000cf3 1212420    user       666        32           1
    ```

    Note the the first is 32768 larger than the last one (from the first part
    of the exercise (1081344 - 1048576 = 32768).  Also note that each subsequent
    queue ID is 32769 greater than the previous:

    ```
    1081344 + 32769 = 1114113
    1114113 + 32769 = 1146882
    1146882 + 32769 = 1179651
    1179651 + 32769 = 1212420
    1212420
    ```

    Interestingly, if I re-run the program from the first part of this exercise,
    it re-uses the queue IDs from the second application:

    ```
    $ ./a.out
    queue_id: 1081344
    queue_id: 1114113
    queue_id: 1146882
    queue_id: 1179651
    queue_id: 1212420
    ```

    And since that program deletes the queues, it cleans up the previously
    un-deleted queues:

	```
    $ ipcs -q

    ------ Message Queues --------
    key        msqid      owner      perms      used-bytes   messages
	```

13. Describe how to build a linked list of data objects in a shared memory
    segment. What would you store as the list pointers?

    I would allocate memory for (1) a free list head index, (2) a used list
    head index, and (3) and array list list nodes.  Each list node would 
    contain a value and the index of the next node in the list.  On initial
    creation, I'd initialize the used list head index to -1, the free list
    head index to 0.  I would then loop over each node in the list, and set
    the next index to the index of the following node.  Node allocation would
    get the node whose index is stored in the free list head index.

    Instead of using pointers, everything that would "normally" be a pointer
    is an index.  This way, when the shared memory segment gets mapped to
    a different virtual address, everything is an offset instead of an absolute
    address.

    The following (also in `exercise_13.c`) shows the basic outline:

    ```c
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
    ```

    Here's some sample runs of the above program:

    ```
    $ ./a.out
    Initializing the memory segment...
    List:
    $ ./a.out add
    List: 186, -1, 17, 42
    $ ./a.out add
    List: 186, -1, 17, 42, 186, -1, 17, 42
    $ ./a.out add
    List: 186, -1, 17, 42, 186, -1, 17, 42, 186, -1, 17, 42
    ```

    Here's the shared memory segment:

    ```
    $ ipcs -m
    
    ------ Shared Memory Segments --------
    key        shmid      owner      perms      bytes      nattch     status
    0x00000cf3 65536      user       600        2080       0
    ```

    Finally, I clean up the memory segment:

    ```
    $ ipcrm -m 65536
    ```

14. Draw a timeline of the program in Figure 15.33 showing the value of the
    variable `i` in both the parent and child, the value of the long integer
    in the shared memory region, and the value returned by the `update`
    function.  Assume that the child runs first after the `fork`.

    ```
    Update always returns the value of i for both the parent and the child.

    +-----------------+-----------------+-----------------+
    | Parent          | Child           | Shared          |
    +-----------------+-----------------+-----------------+
    | i = 0           | -               | 1               |
    | -               | i = 1           | 2               |
    | i = 2           | -               | 3               |
    | -               | i = 3           | 4               |
    | ...             | ...             | ...             |
    | i = 998         | -               | 999             |
    | -               | i = 999         | 1000            |
    +-----------------+-----------------+-----------------+
    ```

15. Redo the program in Figure 15.33 using the XSI shared memory functions from
    Section 15.9 instead of the shared memory-mapped region.

    Here's the updated program (also in `exercise_15.c`):

    ```c
    #include <fcntl.h>
    #include <signal.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <sys/shm.h>
    #include <sys/ipc.h>
    #include <sys/wait.h>
    #include <unistd.h>
    
    #define NLOOPS      1000
    #define SIZE        sizeof(long)
    
    /* TELL/WAIT CHILD/PARENT elided, see exercise_15.c for full program */
    
    /* size of shared memory area */
    static int
    update(long *ptr)
    {
        return((*ptr)++); /* return value before increment */
    }
    
    int
    main(void)
    {
    	int i, counter;
    	pid_t pid;
    	void *area;
    
    	/* picked /etc/passwd because it exists; normally I'd use something else */
    	const key_t key = ftok("/etc/passwd", 0);
    	if (key < 0) {
    		perror("ftok");
    		return 1;
    	}
    
    	/* Create a shared memory segment */
    	const int shm_id = shmget(key, SIZE, IPC_CREAT | 0600);
    	if (shm_id < 0) {
    		perror("shmget");
    		return 1;
    	}
    
    	/* Map the shared memory segment into our address space. */
    	area = shmat(shm_id, NULL, 0);
    	if (area == NULL) {
    		perror("shmat");
    		return 1;
    	}
    
    	TELL_WAIT();
    
    	if ((pid = fork()) < 0) {
    		err_sys("fork error");
    	} else if (pid > 0) {           /* parent */
    		for (i = 0; i < NLOOPS; i += 2) {
    			if ((counter = update((long *)area)) != i)
    				err_quit("parent: expected %d, got %d", i, counter);
    			printf("p: %ld\n", *((long*) area));
    			TELL_CHILD(pid);
    			WAIT_CHILD();
    		}
    
    		wait(NULL);
    	} else {                        /* child */
    		for (i = 1; i < NLOOPS + 1; i += 2) {
    			WAIT_PARENT();
    			if ((counter = update((long *)area)) != i)
    				err_quit("child: expected %d, got %d", i, counter);
    			printf("c: %ld\n", *((long*) area));
    			TELL_PARENT();
    		}
    		exit(0); 
    	}
    
    	/* Unmap the shared memory segment from our address space. */
    	if (shmdt(area) < 0) {
    		perror("shmdt");
    		return 1;
    	}
    
    	/* Delete the shared memory segment */
    	if (shmctl(shm_id, IPC_RMID, NULL) < 0) {
    		perror("shmctl");
    		return 1;
    	}
    
    	exit(0); 
    }
    ```

16. Redo the program in Figure 15.33 using the XSI semaphore functions from
    Section 15.8 to alternate between the parent and the child.

    Skipping.

17. Redo the program in Figure 15.33 using advisory record locking to alternate
    between the parent and the child.

    Skipping.

18. Redo the program in Figure 15.33 using the POSIX semaphore functions from
    Section 15.10 to alternate between the parent and the child.

    I implemented this two ways.  In the first (`exercise18_a.c`) I abused
    a single semaphore to implement strict alternation between the parent
    and the child.  While this approach worked, it was very slow.

    In the second (`exercise18b.c`) I used two named semaphores.  This
    approach was much faster (and I imagine is closer to what the author
    was thinking when writing this question).

    ```c
    #include <fcntl.h>
    #include <semaphore.h>
    #include <signal.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <sys/mman.h>
    #include <sys/wait.h>
    #include <unistd.h>
    
    #define NLOOPS      1000
    #define SIZE        sizeof(long)
    
    #define err_msg(fmt, ...) fprintf(stderr, (fmt "\n"), ##__VA_ARGS__)
    #define err_sys(fmt, ...) fprintf(stderr, (fmt "\n"), ##__VA_ARGS__)
    #define err_quit(fmt, ...) do { err_sys(fmt, ##__VA_ARGS__); exit(1); } while(0)
    
    /* size of shared memory area */
    static int
    update(long *ptr)
    {
        return((*ptr)++); /* return value before increment */
    }
    
    int
    main(void)
    {
    	const char* const parent_semname = "/parent_semaphore";
    	const char* const child_semname = "/child_semaphore";
    
    	int fd, i, counter;
    	pid_t pid;
    	void *area;
    	int sem_value;
    
    	sem_t* const parent_sem = sem_open(parent_semname, O_CREAT, 0600, 1);
    	sem_t* const child_sem  = sem_open(child_semname, O_CREAT, 0600, 0);
    	
    	if (parent_sem == SEM_FAILED || child_sem == SEM_FAILED) {
    		perror("sem_open");
    		return 1;
    	}
    
    	if ((fd = open("/dev/zero", O_RDWR)) < 0)
    		err_sys("open error");
    
    	if ((area = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
    	                 fd, 0)) == MAP_FAILED)
    		err_sys("mmap error");
    
    	close(fd);      /* can close /dev/zero now that it’s mapped */
    
    	if ((pid = fork()) < 0) {
    		err_sys("fork error");
    	} else if (pid > 0) {           /* parent */
    		for (i = 0; i < NLOOPS; i += 2) {
    			sem_wait(parent_sem);
    
    			if ((counter = update((long *)area)) != i)
    				err_quit("parent: expected %d, got %d", i, counter);
    
    			sem_post(child_sem);
    		}
    
    		wait(NULL);
    		sem_close(parent_sem);
    		sem_close(child_sem);
    		sem_unlink(parent_semname);
    		sem_unlink(child_semname);
    	} else {                        /* child */
    		for (i = 1; i < NLOOPS + 1; i += 2) {
    			sem_wait(child_sem);
    
    			if ((counter = update((long *)area)) != i)
    				err_quit("child: expected %d, got %d", i, counter);
    
    			sem_post(parent_sem);
    		}
    	}
    	exit(0); 
    }
    ```
