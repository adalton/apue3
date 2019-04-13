1. Modify the program in Figure 4.3 to use `stat` instead of `lstat`. What
   changes if one of the command-line arguments is a symbolic link?

   After changing `lstat` to `stat`, when the program encounters a symbolic
   link, it reports the type of the file to which the link points instead
   of the type of symbolic link.  See `exercise_1.c`.

2. What happens if the file mode creation mask is set to 777 (octal)? Verify
   the results using your shell’s `umask` command.

   Newly created files and directories are created with mode 000:

```bash
   $ umask 777
   $ touch some_file
   $ mkdir some_directory
   $ ls
   total 4
   d--------- 2 user group 4096 Apr 10 22:23 some_directory
   ---------- 1 user group    0 Apr 10 22:23 some_file
```

3. Verify that turning off user-read permission for a file that you own denies
   your access to the file.

```bash
   $ echo hello > some_file
   $ cat some_file
   hello
   $ chmod u-r some_file
   $ cat some_file
   cat: some_file: Permission denied
```

4. Run the program in Figure 4.9 after creating the files `foo` and `bar`.
   What happens?

   I created two non-empty files, then ran the program from Figure 4.9.
   None of the system calls failed.  The permissions of the existing files
   were not modified, but the files were truncated.

```bash
   $ echo foo > foo
   $ echo bar > bar
   $ ls -l foo bar
   -rw-r----- 1 user group 4 Apr 12 20:05 bar
   -rw-r----- 1 user group 4 Apr 12 20:05 foo
   $ ./a.out
   $ ls -l foo bar
   -rw-r----- 1 user group 0 Apr 12 20:05 bar
   -rw-r----- 1 user group 0 Apr 12 20:05 foo
   $
```

5. In Section 4.12, we said that a file size of 0 is valid for a regular file.
   We also said that the `st_size` field is defined for directories and
   symbolic links. Should we ever see a file size of 0 for a directory or a
   symbolic link?

   _Should we ever see a file size of 0 for a directory?_

   No. A directory will always contain `.` and `..`.
   
   _Should we ever see a file size of 0 for a symbolic link?_

   No. A symbolic link will always contain the path of the file to which
   it references, and that will never be the empty string

6. Write a utility like `cp(1)` that copies a file containing holes, without
   writing the bytes of 0 to the output file.

   I created a simple program, `create_sparse_file.c`, to create a sparse file
   with a couple of characters, 4GB of 0's, and a couple of characters at the
   end.

```
    $ ls -lh sparse_file
    -rw-r----- 1 user group 4.1G Apr 12 21:16 sparse_file

    $ hexdump sparse_file
    0000000 6968 0000 0000 0000 0000 0000 0000 0000
    0000010 0000 0000 0000 0000 0000 0000 0000 0000
    *
    100000000 0000 6f68
    100000004
```

Next, I run the the program in `exercise_6.c` to copy that file:

```
   $ ./a.out sparse_file sparse_file_copy
   $ ls -lh sparse_file*
   -rw-r----- 1 user group 4.1G Apr 12 21:16 sparse_file
   -rw-r----- 1 user group 4.1G Apr 12 21:34 sparse_file_copy
```

I verify that the copy is sparse:

```
   $ hexdump sparse_file_copy
   0000000 6968 0000 0000 0000 0000 0000 0000 0000
   0000010 0000 0000 0000 0000 0000 0000 0000 0000
   *
   100000000 0000 6f68
   100000004
```

And I verify that the files are the same:

```
   $ diff sparse_file sparse_file_copy
   $
```

7. Note in the output from the `ls` command in Section 4.12 that the files
   `core` and `core.copy` have different access permissions. If the umask
   value didn’t change between the creation of the two files, explain how the
   difference could have occurred.
8. When running the program in Figure 4.16, we check the available disk space
   with the `df(1)` command. Why didn’t we use the `du(1)` command?

9. In Figure 4.20, we show the `unlink` function as modifying the changed-status
   time of the file itself. How can this happen?

10. In Section 4.22, how does the system’s limit on the number of open files
    affect the `myftw` function?

11. In Section 4.22, our version of `ftw` never changes its directory. Modify
    this routine so that each time it encounters a directory, it uses the
    `chdir` function to change to that directory, allowing it to use the
    filename and not the pathname for each call to `lstat`. When all the
    entries in a directory have been processed, execute `chdir("..")`.
    Compare the time used by this version and the version in the text.

12. Each process also has a root directory that is used for resolution of
    absolute pathnames. This root directory can be changed with the `chroot`
    function. Look up the description for this function in your manuals.
    When might this function be useful?

13. How can you set only one of the two time values with the `utimes` function?

14. Some versions of the `finger(1)` command output "New mail received ..."
    and "unread since ..." where ... are the corresponding times and dates.
    How can the program determine these two times and dates?

15. Examine the archive formats used by the `cpio(1)` and `tar(1)` commands.
    (These descriptions are usually found in Section 5 of the UNIX Programmer’s
    Manual.) How many of the three possible time values are saved for each
    file? When a file is restored, what value do you think the access time
    is set to, and why?

16. Does the UNIX System have a fundamental limitation on the depth of a
    directory tree? To find out, write a program that creates a directory and
    then changes to that directory, in a loop. Make certain that the length of
    the absolute pathname of the leaf of this directory is greater than your
    system’s `PATH_MAX` limit. Can you call `getcwd` to fetch the directory’s
    pathname? How do the standard UNIX System tools deal with this long
    pathname? Can you archive the directory using either `tar` or `cpio`?

17. In Section 3.16, we described the `/dev/fd` feature. For any user to be
    able to access these files, their permissions must be `rw-rw-rw-`. Some
    programs that create an output file delete the file first, in case it
    already exists, ignoring the return code:

```c
    unlink(path);
    if ((fd = creat(path, FILE_MODE)) < 0)
    	err_sys(...);
```

   What happens if path is `/dev/fd/1`?
