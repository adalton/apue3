1. Refer back to our discussion of the `utmp` and `wtmp` files in Section 6.8.
   Why are the logout records written by the `init` process? Is this handled the
   same way for a network login?

   For console-based logins, `init` manages `agetty` (or something similar),
   so it is the process that gets notified when a that process terminates
   (and that process termination implies a user logout).

   The `init` process isn't involved for network logins, because it is not
   the process that creates and manages the processes associated with the
   login.  The daemon process (e.g., `sshd`) is responsible for updating
   those files in network login cases.

   Consider this example of monitoring the `sysdig` tool during a remote
   login via ssh:

   ```
   $ sudo sysdig fd.filename=wtmp
   21814 15:12:22.674427184 0 sshd (23474) < openat fd=5(<f>/var/log/wtmp) ...
   21823 15:12:22.674433650 0 sshd (23474) > lseek fd=5(<f>/var/log/wtmp) offset=0 whence=2(SEEK_END)
   21824 15:12:22.674434071 0 sshd (23474) < lseek res=1044864
   21825 15:12:22.674434597 0 sshd (23474) > write fd=5(<f>/var/log/wtmp) size=384
   21826 15:12:22.674440028 0 sshd (23474) < write res=384 data=...
   ```

   Note that `sshd` opened and wrote to `/var/log/wtmp`.

2. Write a small program that calls `fork` and has the child create a new
   session. Verify that the child becomes a process group leader and that
   the child no longer has a controlling terminal.
