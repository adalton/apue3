1. As we might guess from Figure 13.2, when the `syslog` facility is
   initialized, either by calling `openlog` directly or on the first call to
   `syslog`, the special device file for the UNIX domain datagram socket,
   `/dev/log`, has to be opened. What happens if the user process (the daemon)
   calls `chroot` before calling `openlog`?

   After the call to `chroot`, an open of `/dev/log` will be relative to the
   new root.  It's possible that `/dev` was bind mounted to the chroot root;
   if it was, then the call to open the device will be successful.  If not,
   then the open will fail.

   The program could open the device before calling `chroot`; open file
   descriptors from outside a chroot are still valid after calling `chroot`.

2. Recall the sample `ps` output from Section 13.2. The only user-level daemon
   that isn't a session leader is the `rsyslogd` process. Explain why the
   `syslogd` daemon isn't a session leader.

   According to
   [this Stack Exchange Answer](https://unix.stackexchange.com/a/333513/90691),
   it was a bug in `rsyslogd`.

3. List all the daemons active on your system, and identify the function of
   each one.

4. Write a program that calls the `daemonize` function in Figure 13.1. After
   calling this function, call `getlogin` (Section 8.15) to see whether the
   process has a login name now that it has become a daemon. Print the results
   to a file.
