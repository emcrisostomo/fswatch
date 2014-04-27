README
======

`fswatch` is a file change monitor that receives notifications when the contents
of the specified files or directories are modified.  fswatch implements four 
kinds of monitors:

  * A monitor based on the _File System Events API_ of Apple OS X.
  * A monitor based on _kqueue_, an event notification interface introduced in
    FreeBSD 4.1 and supported on most *BSD systems (including OS X).
  * A monitor based on _inotify_, a Linux kernel subsystem that reports file
    system changes to applications.
  * A monitor which periodically stats the file system, saves file modification
    times in memory and manually calculates file system changes, which can work
    on any operating system where `stat` (2) can be used.

fswatch should build and work correctly on any system shipping either of the
aforementioned APIs.

Limitations
-----------

The limitations of fswatch depend largely on the monitor being used:

  * The FSEvents monitor, available only on Apple OS X, has no known limitations
    and scales very well with the number of files being observed.
  * The kqueue monitor, available on any *BSD system featuring kqueue, requires
    a file descriptor to be opened for every file being watched.  As a result,
    this monitor scales badly with the number of files being observed and may
    begin to misbehave as soon as the fswatch process runs out of file 
    descriptors.  In this case, fswatch dumps one error on standard error for
    every file that cannot be opened.
  * The inotify monitor, available on Linux since kernel 2.6.13, may suffer a
    queue overflow if events are generated faster than they are read from the
    queue.  In any case, the application is guaranteed to receive an overflow
    notification which can be handled to gracefully recover.  fswatch currently
    throws an exception if a queue overflow occurs.  Future versions will handle
    the overflow by emitting proper notifications.
  * The poll monitor, available on any platform, only relies on available CPU
    and memory to perform its task.  The performance of this monitor degrades
    linearly with the number of files being watched.  

Usage recommendations are as follows:

  * On OS X, use only the FSEvents monitor (which is the default behaviour).
  * On Linux, use the inotify monitor (which is the default behaviour).
  * If the number of files to observe is sufficiently small, use the kqueue
    monitor.  Beware that on some systems the maximum number of file descriptors
    that can be opened by a process is set to a very low value (values as low
    as 256 are not uncommon), even if the operating system may allow a much
    larger value.  In this case, check your OS documentation to raise this limit
    on either a per process or a system-wide basis.
  * If feasible, watch directories instead of watching files.
  * If none of the above applies, use the poll monitor.  The authors' experience
    indicates that fswatch requires approximately 150 MB or RAM memory to 
    observe a hierarchy of 500.000 files with a minimum path length of 32
    characters.  A common bottleneck of the poll monitor is disk access, since
    `stat()`-ing a great number of files may take a huge amount of time.  In this
    case, the latency should be set to a sufficiently large value in order to
    reduce the performance degradation that may result from frequent disk
    access.

Getting fswatch
---------------

The recommended way to get the sources of fswatch in order to build it on your
system is getting a release tarball.  A release tarball contains everything a 
user needs to build fswatch on his system, following the instructions detailed
in the Installation section below and the INSTALL file.

Getting a copy of the source repository is not recommended, unless you are a
developer, you have the GNU Build System installed on your machine and you know
how to boostrap it on the sources.

Installation
------------

See the `INSTALL` file for detailed information about how to configure and
install fswatch.

fswatch is a C++ program and a C++ compiler compliant with the C++11 standard
is required to compile it.  Check your OS documentation for information about
how to install the C++ toolchain and the C++ runtime.

No other software packages or dependencies are required to configure and
install fswatch but the aforementioned APIs used by the file system monitors.

Usage
-----

fswatch accepts a list of paths for which change events should be received:

    $ fswatch [options] ... path-0 ... path-n

The event stream is created even if any of the paths do not exist yet.  If they
are created after fswatch is launched, change events will be properly received.
Depending on the wathcher being used, newly created paths will be monitored
after the amount of configured latency has elapsed.

For more information, refer to the fswatch man page.

Bug Reports
-----------

Bug reports can be sent directly to the authors.

-----

Copyright (C) 2014 Enrico M. Crisostomo

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.