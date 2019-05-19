1. Write a program that calls `tty_raw` and terminates (without resetting the
   terminal mode). If your system provides the `reset(1)` command (all four
   systems described in this text do), use it to restore the terminal mode.

   The following program implements the exercise (also in `exercise_1.c`):

   ```c
   #include <errno.h>
   #include <termios.h>
   #include <unistd.h>
   
   static struct termios save_termios;
   static int ttysavefd = -1;
   static enum { RESET, RAW, CBREAK }  ttystate = RESET;
   
   int
   tty_raw(int fd)
   {
   	int err;
   	struct termios buf;
   
   	if (ttystate != RESET) {
   		errno = EINVAL;
   		return(-1);
   	}
   
   	if (tcgetattr(fd, &buf) < 0)
   		return(-1);
   
   	save_termios = buf; /* structure copy */
   
   	/*
   	 * Echo off, canonical mode off, extended input
   	 * processing off, signal chars off.
   	 */
   	buf.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
   
   	/*
   	 * No SIGINT on BREAK, CR-to-NL off, input parity
   	 * check off, don’t strip 8th bit on input, output
   	 * flow control off.
   	 */
   	buf.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
   
   	/*
   	 * Clear size bits, parity checking off.
   	 */
   	buf.c_cflag &= ~(CSIZE | PARENB);
   
   	/*
   	 * Set 8 bits/char.
   	 */
   	buf.c_cflag |= CS8;
   
   	/*
   	 * Output processing off.
   	 */
   	buf.c_oflag &= ~(OPOST);
   
   	/*
   	 * Case B: 1 byte at a time, no timer.
   	 */
   	buf.c_cc[VMIN] = 1;
   	buf.c_cc[VTIME] = 0;
   
   	if (tcsetattr(fd, TCSAFLUSH, &buf) < 0)
   		return(-1);
   
   	/*
   	 * Verify that the changes stuck.  tcsetattr can return 0 on
   	 * partial success.
   	 */
   	if (tcgetattr(fd, &buf) < 0) {
   		err = errno;
   		tcsetattr(fd, TCSAFLUSH, &save_termios);
   		errno = err;
   		return(-1);
   	}
   
   	if ((buf.c_lflag & (ECHO | ICANON | IEXTEN | ISIG)) ||
   	    (buf.c_iflag & (BRKINT | ICRNL | INPCK | ISTRIP | IXON)) ||
   	    (buf.c_cflag & (CSIZE | PARENB | CS8)) != CS8 ||
   	    (buf.c_oflag & OPOST) || buf.c_cc[VMIN] != 1 ||
   	    buf.c_cc[VTIME] != 0) {
   		/*
   		 * Only some of the changes were made.  Restore the
   		 * original settings.
   		 */
   		tcsetattr(fd, TCSAFLUSH, &save_termios);
   		errno = EINVAL;
   		return(-1);
   	}
   	ttystate = RAW;
   	ttysavefd = fd;
   
   	return(0);
   }
   
   int
   main(void)
   {
   	tty_raw(1);
   	return 0;
   }
   ```

   After running the program, output to the terminal is raw mode.  Using the
   `reset` command puts the terminal back in a sane state.

2. The `PARODD` flag in the `c_cflag` field allows us to specify even or odd
   parity. The BSD `tip` program, however, also allows the parity bit to be 0
   or 1. How does it do this?

   Skipping

3. If your system’s `stty(1)` command outputs the MIN and TIME values, do the
   following exercise. Log in to the system twice and start the `vi` editor
   from one login. Use the `stty` command from your other login to determine
   which values `vi` sets MIN and TIME to (since `vi` sets the terminal to
   noncanonical mode). (If you are running a windowing system on your terminal,
   you can do this same test by logging in once and using two separate windows
   instead.)

   1. Linux  
      Terminal 1:
      ```
      $ who am i
      user  pts/0       2019-05-17 20:32 ...

      $ stty -a
      ...  min = 1; time = 0; ...

      $ vi
      ```

      Terminal 2:
      ```
      $ stty -a --file /dev/pts/0
      ... min = 1; time = 0; ...
      ```
      Here `vi` is `vim`.  With Linux, logged in remotely via ssh, I don't see the
      values of `MIN` or `TIME` change when running `vi`.

   2. MacOS  
      Terminal 1:
      ```
      $ who am i
      user  ttys001       May 19 14:03

      $ stty -a
      ... min = 1; time = 0; ...

      $ vi
      ```

      Terminal 2:
      ```
      $ stty -a < /dev/ttys001
      ...; min = 1; time = 0; ...
      ```

      Again, here, `vi` is `vim.  With MacOS, logged into via a terminal
      application, I don't see the values of `MIN` or `TIME` change when
      running `vi`.

   3. FreeBSD  
      Terminal 1:
      ```
      $ who am i
      user   ttyv0        May 19 14:09

      $ stty -a
      ... min = 1; time = 0; ...

      $ vi
      ```

      Terminal 2:
      ```
      $ stty -a < /dev/ttyv0
      ... min = 1; time = 0; ...
      ```

      Again, here, `vi` is `vim.  With FreeBSD, logged into the console, I don't
      see the values of `MIN` or `TIME` change when running `vi`.

   My best guess here is that either `vim` behaves differently from `vi`
   (assuming that the author meant `vi` when he wrote `vim`), or that it
   has chanced since the text was written.
