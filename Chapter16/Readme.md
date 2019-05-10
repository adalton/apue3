1. Write a program to determine your system’s byte ordering.

   ```c
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
   ```

2. Write a program to print out which `stat` structure members are supported
   for sockets on at least two different platforms, and describe how the results
   differ.

   Here, I interpreted "socket" as "TCP Socket".

   ```c
   #include <stdio.h>
   #include <sys/socket.h>
   #include <sys/stat.h>
   #include <sys/types.h>
   #include <unistd.h>
   
   int
   main(void)
   {
   	struct stat statbuf = {};
   	const int fd = socket(AF_INET, SOCK_STREAM, 0);
   	if (fd < 0) {
   		perror("socket");
   		return 1;
   	}
   
   	if (fstat(fd, &statbuf) < 0) {
   		perror("fstat");
   		return 1;
   	}
   
   	printf("st_dev:     %lu\n", statbuf.st_dev);
   	printf("st_ino:     %lu\n", statbuf.st_ino);
   	printf("st_nlink:   %lu\n", statbuf.st_nlink);
   	printf("st_mode:    %u\n",  statbuf.st_mode);
   	printf("st_uid:     %u\n",  statbuf.st_uid);
   	printf("st_gid:     %u\n",  statbuf.st_gid);
   	printf("st_rdev:    %lu\n", statbuf.st_rdev);
   	printf("st_size:    %lu\n", statbuf.st_size);
   	printf("st_blksize: %lu\n", statbuf.st_blksize);
   	printf("st_blocks:  %lu\n", statbuf.st_blocks);
   	printf("st_atime:   %lu\n", statbuf.st_atime);
   	printf("st_mtime:   %lu\n", statbuf.st_mtime);
   	printf("st_ctime:   %lu\n", statbuf.st_ctime);
   
   	close(fd);
   	return 0;
   }
   ```

   On Linux, I get:

   ```
   st_dev:     8
   st_ino:     19338191
   st_nlink:   1
   st_mode:    49663
   st_uid:     1000
   st_gid:     1000
   st_rdev:    0
   st_size:    0
   st_blksize: 4096
   st_blocks:  0
   st_atime:   0
   st_mtime:   0
   st_ctime:   0
   ```

   On MacOS, I get:

   ```
   st_dev:     0
   st_ino:     0
   st_nlink:   0
   st_mode:    49590
   st_uid:     501
   st_gid:     20
   st_rdev:    0
   st_size:    0
   st_blksize: 131072
   st_blocks:  0
   st_atime:   0
   st_mtime:   0
   st_ctime:   0
   ```
   On FreeBSD, I get:

   ```
   st_dev:     0
   st_ino:     0
   st_nlink:   0
   st_mode:    49590
   st_uid:     1000
   st_gid:     1000
   st_rdev:    0
   st_size:    0
   st_blksize: 32768
   st_blocks:  0
   st_atime:   0
   st_mtime:   0
   st_ctime:   0
   ```

   All three reported `st_mode`, `st_uid`, `st_gid`, and `st_blksize`; the
   `st_uid` and `st_gid` fields were consistently the UID and GID of the
   user who ran the program.

   In addition to those fields, Linux also reported `st_dev`, `st_ino`,
   and `st_nlink`.

3. The program in Figure 16.17 provides service on only a single endpoint.
   Modify the program to support service on multiple endpoints (each with a
   different address) at the same time.

4. Write a client program and a server program to return the number of processes
   currently running on a specified host computer.

5. In the program in Figure 16.18, the server waits for the child to execute
   the `uptime` command and exit before accepting the next connect request.
   Redesign the server so that the time to service one request doesn’t delay
   the processing of incoming connect requests.

6. Write two library routines: one to enable asynchronous (signal-based) I/O
   on a socket and one to disable asynchronous I/O on a socket. Use Figure
   16.23 to make sure that the functions work on all platforms with as many
   socket types as possible.
