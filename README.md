README
======

This is a small program using the Mac OS X FSEvents API to monitor a directory.
When an event about any change to that directory is received, the specified
shell command is executed by `/bin/bash`.

If you're on GNU/Linux, [inotifywatch][inw] (part of the `inotify-tools`
package on most distributions) provides similar functionality.

[inw]: http://linux.die.net/man/1/inotifywatch

Installation
------------

You can install fswatch using brew:

    $ brew install fswatch

Compile
-------

The recommended way to get the sources of fswatch in order to build it on your
system is getting a release tarball.  A release tarball contains everything a 
user needs to build fsw on his system, following the instructions detailed in
the INSTALL file.

Getting a copy of the source repository is not recommended, unless you are a
developer, you have the GNU Build System installed on your machine and you know
how to boostrap it on the sources.

A script called `autogen.sh` is included to boostrap the GNU Build System
on a development machine.

### Basic Usage

    ./fswatch /some/dir "echo changed" 

This would monitor `/some/dir` for any change, and run `echo changed`
when a modification event is received.

In the case you want to watch multiple directories, just separate them
with colons like:

    ./fswatch /some/dir:/some/otherdir "echo changed" 

### Usage with rsync

`fswatch` can be used with `rsync` to keep a remote directory in sync
with a local directory continuously as local files change.  The
following example was contributed by
[Michael A. Smith](http://twitter.com/michaelasmith):

```bash
#!/bin/sh

##
# Keep local path in sync with remote path on server.
# Ignore .git metadata.
#
local=$1
remote=$2

cd "$local" &&
fswatch . "date +%H:%M:%S && rsync -iru --exclude .git --exclude-from=.gitignore --delete . $remote"
```

### About

This code was adapted from the example program in the
[FSEvents API documentation](https://developer.apple.com/library/mac/documentation/Darwin/Conceptual/FSEvents_ProgGuide/FSEvents_ProgGuide.pdf).
