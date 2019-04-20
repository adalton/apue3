1. Refer back to our discussion of the `utmp` and `wtmp` files in Section 6.8.
   Why are the logout records written by the `init` process? Is this handled the
   same way for a network login?

2. Write a small program that calls `fork` and has the child create a new
   session. Verify that the child becomes a process group leader and that
   the child no longer has a controlling terminal.
