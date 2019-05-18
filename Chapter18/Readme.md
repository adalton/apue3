1. Write a program that calls `tty_raw` and terminates (without resetting the
   terminal mode). If your system provides the `reset(1)` command (all four
   systems described in this text do), use it to restore the terminal mode.

2. The `PARODD` flag in the `c_cflag` field allows us to specify even or odd
   parity. The BSD `tip` program, however, also allows the parity bit to be 0
   or 1. How does it do this?

3. If your system’s `stty(1)` command outputs the MIN and TIME values, do the
   following exercise. Log in to the system twice and start the `vi` editor
   from one login. Use the `stty` command from your other login to determine
   which values `vi` sets MIN and TIME to (since `vi` sets the terminal to
   noncanonical mode). (If you are running a windowing system on your terminal,
   you can do this same test by logging in once and using two separate windows
   instead.)
