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

   When a core dump occurs, the kernel writes the core file, not the process.
   The kernel may not respect the process' umask when writing the core file.
   The copy was created by a userspace process, and hence, respected the
   umask.

8. When running the program in Figure 4.16, we check the available disk space
   with the `df(1)` command. Why didn’t we use the `du(1)` command?

   The `du(1)` command estimates disk usage by iterating over the contents
   of a given directory and collecting the size of each entry.  Since the
   program Figure 4.16 unlinked the file, the `du(1)` command would not find
   it when it searched the directory (even during the period while the file
   is open without a filename).

9. In Figure 4.20, we show the `unlink` function as modifying the changed-status
   time of the file itself. How can this happen?

   The `unlink` system call removes a link (a name to file mapping) from a
   file.  When there is only one link, the file is removed.  If there are
   multiple links, then unlinking a name does not remove the file, only the
   filename entry in the directory.  In that case, `unlink` updates the
   changed-status time of the file to which the name referenced.

   Consider the following:

   * Create a file

   ```
   $ touch a
   ```

   * Create a second link to the same file

   ```
   $ ln a b
   ```

   *  View the file status of the original file

   ```
   $ stat a
     File: a
     Size: 0         	Blocks: 0          IO Block: 4096   regular empty file
   Device: 800h/2048d	Inode: 439741      Links: 2
   Access: (0640/-rw-r-----)  Uid: ( 1000/ user)   Gid: ( 1000/ group)
   Access: 2019-04-13 21:47:51.295729076 -0400
   Modify: 2019-04-13 21:47:51.295729076 -0400
   Change: 2019-04-13 21:47:54.049062408 -0400
    Birth: -
   ```

   * Remove the second link

   ```
   $ rm b
   ```

   * Reexamine the file status of the original file.  Notice that the
     changed-status is updated.

   ```
   $ stat a
     File: a
     Size: 0         	Blocks: 0          IO Block: 4096   regular empty file
   Device: 800h/2048d	Inode: 439741      Links: 1
   Access: (0640/-rw-r-----)  Uid: ( 1000/ user)   Gid: ( 1000/ group)
   Access: 2019-04-13 21:47:51.295729076 -0400
   Modify: 2019-04-13 21:47:51.295729076 -0400
   Change: 2019-04-13 21:48:00.462395742 -0400
    Birth: -
   ```

10. In Section 4.22, how does the system’s limit on the number of open files
    affect the `myftw` function?

    The `myftw` function calls `dopath`.  The `dopath` function calls
    `opendir`, which allocates a file descriptor.  With that file descriptor
    open, `dopath` iterates over the contents of the directory, and
    recursively calls itself for each directory that it finds.  As a result,
    it can try to open as many file descriptors as the max path depth.

11. In Section 4.22, our version of `ftw` never changes its directory. Modify
    this routine so that each time it encounters a directory, it uses the
    `chdir` function to change to that directory, allowing it to use the
    filename and not the pathname for each call to `lstat`. When all the
    entries in a directory have been processed, execute `chdir("..")`.
    Compare the time used by this version and the version in the text.

    Note: I skipped this question

12. Each process also has a root directory that is used for resolution of
    absolute pathnames. This root directory can be changed with the `chroot`
    function. Look up the description for this function in your manuals.
    When might this function be useful?

    The `chroot` function is useful in a number of scenarios, I'll describe a
    couple.  The system call is useful in system recovery scenarios.  Consider
    a secnario where a Linux machine will not boot, perhaps because of some
    previous misconfiguration.  A person can boot the machine off of some
    external bootable media (e.g., DVD, USB) and get a root shell.  The person
    can then mount the "real" root partition of the machine (as well as any
    other needed mount points) and then use the `chroot` command --- which
    uses the `chroot` system call --- to get a shell with the root directory
    referring to the "real" root of the machine.  The use can then issue
    commands installed on the machine to repair the machine's configuraiton.

    The command can also be used to give processes some security isolation.
    When a process is run in a `chroot` environment, it cannot (easily) access
    files/directories outside of its root directory.

13. How can you set only one of the two time values with the `utimes` function?

    The `utimes` system call updates both the access and modification times
    of the given file.  There is no direct way to modify only one of those
    values with `utimes`.  To simulate changing only one value, you could
    first determine the existing value using the `stat` system call, and
    then specify the same time value in the call to `utimes`.

14. Some versions of the `finger(1)` command output "New mail received ..."
    and "unread since ..." where ... are the corresponding times and dates.
    How can the program determine these two times and dates?

    The `finger(1)` program can `stat` the mail spool file for the user.
    If the user has new mail, then the modification time will be more recent
    than the access time.  The access time will reflect the last time that
    the user read his or her mail.

15. Examine the archive formats used by the `cpio(1)` and `tar(1)` commands.
    (These descriptions are usually found in Section 5 of the UNIX Programmer’s
    Manual.) How many of the three possible time values are saved for each
    file? When a file is restored, what value do you think the access time
    is set to, and why?

    _How many of the three possible time values are saved for each file?_

    Only the modification time.  The access time would be modified by the
    process creating the archive, so the value wouldn't be very meanful.
    
    _When a file is restored, what value do you think the access time is set
    to, and why?_

    Both the access and change times are set to the time at which the file
    was re-created by extracting the archive.  Since these times are not
    stored in the archive, the current time would be the default assigned
    to them when the file is created.

16. Does the UNIX System have a fundamental limitation on the depth of a
    directory tree? To find out, write a program that creates a directory and
    then changes to that directory, in a loop. Make certain that the length of
    the absolute pathname of the leaf of this directory is greater than your
    system’s `PATH_MAX` limit. Can you call `getcwd` to fetch the directory’s
    pathname? How do the standard UNIX System tools deal with this long
    pathname? Can you archive the directory using either `tar` or `cpio`?

    _Does the UNIX System have a fundamental limitation on the depth of a
    directory tree?_

    No, the directory abstraction does not place any limit on the depth of
    a directory tree.  With the program in `exercise_16.c` I was able to
    create a directory whose path length is greater than `PATH_MAX`:

    ```
    $ pwd | wc -c
    8092
    ```

    Note that I had to break it up with calls to `mkdir` and `chdir` with
    relative paths. In an previous revision of this program, I tried to
    use only `mkdir` with ever-increasing paths lengths, and that failed
    once the path length exceeded `PATH_MAX`.

    _Can you call `getcwd` to fetch the directory’s pathname?_

    Yes, on Linux with glibc-2.28, `getcwd` is successful when the current
    working directory is greater than `PATH_MAX` (as long as the given buffer
    is large enough to hold the path).
    
    _How do the standard UNIX System tools deal with this long pathname?
    Can you archive the directory using either `tar` or `cpio`?_

    Some tools that depend on the path fail because the path is too long.  In
    general, these applications need to allocate memory buffers to store
    paths, and they don't want to make that allocation complex by dynamically
    handling large paths.

    I can create an archive using `tar`, but I cannot extract that archive:

    ```
    Cannot mkdir: File name too long
    tar: Exiting with failure status due to previous errors
    ```

    I was, however, able to use `rm -rf` and specifying the top-level directory
    to remove them, so `rm` was able to deal with the path length greater
    than `PATH_MAX`.

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

    Note that in Linux `/dev/fd` is a symbolic link to `/proc/self/fd`, and
    `/proc/self` is a symbol link to `/proc/<pid>/`, so we're really talking
    about `/proc/<pid>/fd/1`.  That directory does not have write permission
    for the user, and we cannot assign that permission:

    ```
    $ chmod u+w /proc/self/fd
    chmod: changing permissions of '/proc/self/fd': Operation not permitted
    ```

    So, there is no way that the call to `unlink` will be successful (and
    the code silently ignores that failure).

    The target file is a symbol link to the file corresponding to the standard
    output of the process.  If run from a shell, that'll be a character
    device file corresponding to I/O device for the terminal session.  If
    standard output redirection has occurred, then the link will point to
    the target of that redirection.

    We've previously seen that a call to `creat` for an existing file will
    not fail or change the file's permissions, but will simply truncate
    the file.  Truncating the character device file has no effect.  If
    standard output has been redirected to a file, then that file will be
    truncated.

    ```
    $ echo hi > out
    $ ./a.out > out
    unlink: Operation not permitted
    $ cat out
    $
    ```
