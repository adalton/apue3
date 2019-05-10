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

2. Write a program to print out which stat structure members are supported for
   sockets on at least two different platforms, and describe how the results
   differ.

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
