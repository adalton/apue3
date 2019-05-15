1. We chose to use UNIX domain datagram sockets in Figure 17.3, because they
   retain message boundaries. Describe the changes that would be necessary to
   use regular pipes instead. How can we avoid copying the messages two extra
   times?

   _Describe the changes that would be necessary to use regular pipes instead._

   To use regular pipes, one option would be to write the message length
   followed by the message content:

   ```
    +----------------+
    | message length |
    +----------------+
    | message body   |
    +----------------+
   ```

   Here's the implementation (also in `exercise_1a.c`):

   ```c
   #include <poll.h>
   #include <pthread.h>
   #include <stdlib.h>
   #include <stdio.h>
   #include <string.h>
   #include <sys/msg.h>
   #include <sys/socket.h>
   #include <unistd.h>
   
   #define err_sys(fmt, ...) fprintf(stderr, (fmt "\n"), ##__VA_ARGS__)
   #define err_exit(err, fmt, ...) do { err_sys(fmt, ##__VA_ARGS__); exit(err); } while(0)
   #define err_quit(fmt, ...) err_exit(1, fmt, ##__VA_ARGS__)
   
   #define NQ         3 /* number of queues */
   #define MAXMSZ   512 /* maximum message size */
   #define KEY    0x123 /* key for first message queue */
   
   struct threadinfo {
   	int qid;
   	int fd;
   };
   
   struct mymesg {
   	long mtype;
   	char mtext[MAXMSZ];
   };
   
   void*
   helper(void* arg)
   {
   	int n;
   	struct mymesg m;
   	struct threadinfo* tip = arg;
   
   	for (;;) {
   		memset(&m, 0, sizeof(m));
   
   		if ((n = msgrcv(tip->qid, &m, MAXMSZ, 0, MSG_NOERROR)) < 0)
   			err_sys("msgrcv error");
   
   		/* added for exercise */
   		if (write(tip->fd, &n, sizeof(n)) < 0)
   			err_sys("write error");
   
   		if (write(tip->fd, m.mtext, n) < 0)
   			err_sys("write error");
   	}
   }
   
   int
   main(void)
   {
   	int i, n, err;
   	int fd[2];
   	int qid[NQ];
   	struct pollfd pfd[NQ];
   	struct threadinfo ti[NQ];
   	pthread_t tid[NQ];
   	char buf[MAXMSZ];
   
   	for (i = 0; i < NQ; ++i) {
   		if ((qid[i] = msgget((KEY + i), IPC_CREAT | 0666)) < 0)
   			err_sys("msgget error");
   
   		printf("queue ID %d is %d\n", i, qid[i]);
   
   		if (pipe(fd) < 0) /* changed for exercise */
   			err_sys("pipe error");
   
   		pfd[i].fd = fd[0];
   		pfd[i].events = POLLIN;
   		ti[i].qid = qid[i];
   		ti[i].fd = fd[1];
   
   		if ((err = pthread_create(&tid[i], NULL, helper, &ti[i])) != 0)
   			err_exit(err, "pthread_create error");
   	}
   
   	for (;;) {
   		if (poll(pfd, NQ, -1) < 0)
   			err_sys("poll error");
   
   		for (i = 0; i < NQ; ++i) {
   			if (pfd[i].revents & POLLIN) {
   				int length; /* added for exercise */
   
   				/* added for exercise */
   				if (read(pfd[i].fd, &length, sizeof(length)) < 0)
   					err_sys("read error");
   
   				/* changed for exercise */
   				if ((n = read(pfd[i].fd, buf, length)) < 0)
   					err_sys("read error");
   
   				buf[n] = '\0';
   				printf("queue id %d, message %s\n", qid[i], buf);
   			}
   		}
   	}
   
   	exit(0);
   }
   ```

   The main thread can then perform two read operations: (1) read the fixed-size
   message length, then (2) read the number of bytes from (1).  This will
   ensure that only the single message is read from the pipe.

   _How can we avoid copying the messages two extra times?_

   One option would be to dynamically allocate the message, read message from
   the queue into the dynamically allocated message, then write the address
   of the dynamically allocated message to the socket.  The main thread could
   then read that address from the socket (or pipe) and access the message
   without having to copy it.  The main thread would then free the memory
   (or otherwise make it available for recycling).

   Here's an extension of the solution to the first part of this exercise
   that implements this option (also in `exercise_1b.c`).  While this uses
   a pipe instead of a socket, the approach would also work with the socket.

   ```c
   #include <poll.h>
   #include <pthread.h>
   #include <stdlib.h>
   #include <stdio.h>
   #include <string.h>
   #include <sys/msg.h>
   #include <sys/socket.h>
   #include <unistd.h>
   
   #define err_sys(fmt, ...) fprintf(stderr, (fmt "\n"), ##__VA_ARGS__)
   #define err_exit(err, fmt, ...) do { err_sys(fmt, ##__VA_ARGS__); exit(err); } while(0)
   #define err_quit(fmt, ...) err_exit(1, fmt, ##__VA_ARGS__)
   
   #define NQ         3 /* number of queues */
   #define MAXMSZ   512 /* maximum message size */
   #define KEY    0x123 /* key for first message queue */
   
   struct threadinfo {
   	int qid;
   	int fd;
   };
   
   struct mymesg {
   	long mtype;
   	char mtext[MAXMSZ];
   };
   
   void*
   helper(void* arg)
   {
   	int n;
   	struct mymesg* m;
   	struct threadinfo* tip = arg;
   
   	for (;;) {
   		/* Added for exercise */
   		m = malloc(sizeof(struct mymesg));
   		if (m == NULL)
   			err_sys("malloc error");
   
   		if ((n = msgrcv(tip->qid, m, MAXMSZ, 0, MSG_NOERROR)) < 0)
   			err_sys("msgrcv error");
   
   		if (write(tip->fd, &m, sizeof(m)) < 0)
   			err_sys("write error");
   	}
   }
   
   int
   main(void)
   {
   	int i, n, err;
   	int fd[2];
   	int qid[NQ];
   	struct pollfd pfd[NQ];
   	struct threadinfo ti[NQ];
   	pthread_t tid[NQ];
   	char buf[MAXMSZ];
   
   	for (i = 0; i < NQ; ++i) {
   		if ((qid[i] = msgget((KEY + i), IPC_CREAT | 0666)) < 0)
   			err_sys("msgget error");
   
   		printf("queue ID %d is %d\n", i, qid[i]);
   
   		if (pipe(fd) < 0) /* changed for exercise */
   			err_sys("pipe error");
   
   		pfd[i].fd = fd[0];
   		pfd[i].events = POLLIN;
   		ti[i].qid = qid[i];
   		ti[i].fd = fd[1];
   
   		if ((err = pthread_create(&tid[i], NULL, helper, &ti[i])) != 0)
   			err_exit(err, "pthread_create error");
   	}
   
   	for (;;) {
   		if (poll(pfd, NQ, -1) < 0)
   			err_sys("poll error");
   
   		for (i = 0; i < NQ; ++i) {
   			if (pfd[i].revents & POLLIN) {
   				struct mymesg* m; /* Added for exercise */
   
   				/* changed for exercise */
   				if ((n = read(pfd[i].fd, &m, sizeof(m))) < 0)
   					err_sys("read error");
   
   				printf("queue id %d, message %s\n", qid[i], m->mtext);
   
   				/* Added for exercise */
   				free(m);
   			}
   		}
   	}
   
   	exit(0);
   }
   ```

2. Write the following program using the file descriptor passing functions from
   this chapter and the parent–child synchronization routines from Section 8.9.
   The program calls `fork`, and the child opens an existing file and passes
   the open descriptor to the parent. The child then positions the file using
   `lseek` and notifies the parent. The parent reads the file’s current offset
   and prints it for verification. If the file was passed from the child to the
   parent as we described, they should be sharing the same file table entry, so
   each time the child changes the file’s current offset, that change should
   also affect the parent’s descriptor. Have the child position the file to a
   different offset and notify the parent again.

3. In Figures 17.20 and 17.21, we differentiated between declaring and defining
   the global variables. What is the difference?

   _Defining_ a variable allocates storage for the variable.
   _Declaring_ a variable communicates to the compiler that a symbol with
   the given name and type exists, but does not allocate storage for it.

   When a variable will be used by more than one compilation unit
   (source file), it is _defined_ in one file and _declared_ in the others.
   The location of the _declared_ symbols is resolved at link time.

4. Recode the `buf_args` function (Figure 17.23), removing the compile-time
   limit on the size of the `argv` array. Use dynamic memory allocation.

5. Describe ways to optimize the function `loop` in Figure 17.29 and
   Figure 17.30.  Implement your optimizations.

6. In the `serv_listen` function (Figure 17.8), we `unlink` the name of the file
   representing the UNIX domain socket if the file already exists. To avoid
   unintentionally removing a file that isn’t a socket, we could call `stat`
   first to verify the file type. Explain the two problems with this approach.

7. Describe two possible ways to pass more than one file descriptor with a
   single call to `sendmsg`. Try them out to see if they are supported by your
   operating system.
