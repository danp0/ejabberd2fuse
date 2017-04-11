# ejabberd2fuse

An ejabberd virtual file system.

## Building

You will need cmake to build this project. You will
need to install the fuse library and ejabberd. Then
you can type *make* to build the application.

## Usage

Mount a virtual file system into ejabberd. 

Copy the *.erlang.cookie* from ejabberd to ejabberd2fuse directory.
Change the ownership to the current user.

Enter the following command to start the file system:
```
  $ ./ejabberd2fuse.sh <directory>

  where <directory> is the mount point for the ejabberd 
  file system.
```
If the command fails, you may need to add your user to the
*fuse* group.

Enter the following command to unmount the file system:
```
  $ ./umount.sh <directory>

  where <directory> is the mount point above.
```

## Examples

First, mount the file system:

```
  $ ./ejabberd2fuse.sh /tmp/e2f
  FUSE library version: 2.8.3
  nullpath_ok: 0
  unique: 1, opcode: INIT (26), nodeid: 0, insize: 56
  INIT: 7.14
  flags=0x0000f07b
  max_readahead=0x00020000
     INIT: 7.12
     flags=0x00000011
     max_readahead=0x00020000
     max_write=0x00020000
     unique: 1, success, outsize: 40
```

Then access the mounted file system:

```
  $ ls -l /tmp/e2f/
  total 0
  drwxr-xr-x 2 root root 0 Apr 10 15:55 server
  drwxr-xr-x 2 root root 0 Apr 10 15:55 user

  $ ls -l /tmp/e2f/server/
  total 0
  -r--r--r-- 1 root root 2177 Apr 10 15:55 mnesia
  -r--r--r-- 1 root root   97 Apr 10 15:55 status

  $ ls -l /tmp/e2f/user/
  total 0
  -r--r--r-- 1 root root 84 Apr 10 15:55 connected

  $ more /tmp/e2f/server/status
  The node ejabberd@greenlantern is started. Status: startedejabberd 2.1.13 is running in that node

  $ more /tmp/e2f/server/mnesia
  [
    {access_module,mnesia}
    {auto_repair,true}
    {backup_module,mnesia_backup}
  ...

  $ more /tmp/e2f/user/connected
  dan@example.com/work
  bill@example.com/work

```

Then unmount the file system:

```
  $ ./umount.sh /tmp/e2f/
```

### Bugs
...

## License

Copyright © 2017 D. Pollind

Distributed under the Eclipse Public License either version 1.0 or (at
your option) any later version.

