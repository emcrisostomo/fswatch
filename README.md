[![License](https://img.shields.io/badge/license-GPL--3.0-blue.svg?style=flat)](https://github.com/emcrisostomo/fswatch/blob/master/COPYING)
[![Build Status](https://travis-ci.org/emcrisostomo/fswatch.svg?branch=master)](https://travis-ci.org/emcrisostomo/fswatch)

README
======

`fswatch` is a file change monitor that receives notifications when the contents
of the specified files or directories are modified.  `fswatch` implements
several monitors:

  * A monitor based on the _File System Events API_ of Apple OS X.
  * A monitor based on _kqueue_, a notification interface introduced in FreeBSD
    4.1 (and supported on most *BSD systems, including OS X).
  * A monitor based on the _File Events Notification_ API of the Solaris kernel
    and its derivatives.
  * A monitor based on _inotify_, a Linux kernel subsystem that reports file
    system changes to applications.
  * A monitor based on _ReadDirectoryChangesW_, a Microsoft Windows API that
    reports changes to a directory.
  * A monitor which periodically stats the file system, saves file modification
    times in memory, and manually calculates file system changes (which works
    anywhere `stat (2)` can be used).

`fswatch` should build and work correctly on any system shipping either of the
aforementioned APIs.

Table of Contents
-----------------

  * [libfswatch](#libfswatch)
  * [Features](#features)
  * [Limitations](#limitations)
  * [Getting fswatch](#getting-fswatch)
  * [Building from Source](#building-from-source)
  * [Installation](#installation)
  * [Documentation](#documentation)
  * [Localization](#localization)
  * [Usage](#usage)
  * [Contributing](#contributing)
  * [Bug Reports](#bug-reports)

libfswatch
----------

`fswatch` is a frontend of `libfswatch`, a library with C and C++ binding.  More
information on `libfswatch` can be found [here][README.libfswatch.md]. 

[README.libfswatch.md]: README.libfswatch.md

Features
--------

`fswatch` main features are:

  * Support for many OS-specific APIs such as kevent, inotify, and FSEvents.
  * Recursive directory monitoring.
  * Path filtering using including and excluding regular expressions.
  * Customizable record format.
  * Support for periodic idle events.

Limitations
-----------

The limitations of `fswatch` depend largely on the monitor being used:

  * The **FSEvents** monitor, available only on OS X, has no known limitations,
    and scales very well with the number of files being observed.

  * The **File Events Notification** monitor, available on Solaris kernels and
    its derivatives, has no known limitations.

  * The **kqueue** monitor, available on any \*BSD system featuring kqueue,
    requires a file descriptor to be opened for every file being watched.  As a
    result, this monitor scales badly with the number of files being observed,
    and may begin to misbehave as soon as the `fswatch` process runs out of file
    descriptors.  In this case, `fswatch` dumps one error on standard error for
    every file that cannot be opened.

  * The **inotify** monitor, available on Linux since kernel 2.6.13, may suffer
    a queue overflow if events are generated faster than they are read from the
    queue.  In any case, the application is guaranteed to receive an overflow
    notification which can be handled to gracefully recover.  `fswatch`
    currently throws an exception if a queue overflow occurs.  Future versions
    will handle the overflow by emitting proper notifications.

  * The **Windows** monitor can only establish a watch _directories_, not files.
    To watch a file, its parent directory must be watched in order to receive
    change events for all the directory's children, _recursively_ at any depth.
    Optionally, change events can be filtered to include only changes to the
    desired file.

  * The **poll** monitor, available on any platform, only relies on
    available CPU and memory to perform its task.  The performance of this
    monitor degrades linearly with the number of files being watched.

Usage recommendations are as follows:

  * On OS X, use only the `FSEvents` monitor (which is the default behaviour).

  * On Solaris and its derivatives use the _File Events Notification_ monitor.

  * On Linux, use the `inotify` monitor (which is the default behaviour).

  * If the number of files to observe is sufficiently small, use the `kqueue`
    monitor.  Beware that on some systems the maximum number of file descriptors
    that can be opened by a process is set to a very low value (values as low as
    256 are not uncommon), even if the operating system may allow a much larger
    value.  In this case, check your OS documentation to raise this limit on
    either a per process or a system-wide basis.

  * If feasible, watch directories instead of files.  Properly crafting the
    receiving side of the events to deal with directories may sensibly reduce
    the monitor resource consumption.

  * On Windows, use the `windows` monitor.

  * If none of the above applies, use the poll monitor.  The authors' experience
    indicates that `fswatch` requires approximately 150 MB of RAM memory to
    observe a hierarchy of 500.000 files with a minimum path length of 32
    characters.  A common bottleneck of the poll monitor is disk access, since
    `stat()`-ing a great number of files may take a huge amount of time.  In
    this case, the latency should be set to a sufficiently large value in order
    to reduce the performance degradation that may result from frequent disk
    access.

Getting fswatch
---------------

A regular user may be able to fetch `fswatch` from the package manager of your
OS or a third-party one.  If you are looking for `fswatch` for OS X, you can
install it using either [MacPorts] or [Homebrew]:

```
# MacPorts
$ port install fswatch

# Homebrew
$ brew install fswatch
```

Check your favourite package manager and let us know if `fswatch` is missing
there.

[MacPorts]: https://www.macports.org
[Homebrew]: http://brew.sh

Building from Source
--------------------

A user who wishes to build `fswatch` should get a [release tarball][release].
A release tarball contains everything a user needs to build `fswatch` on their
system, following the instructions detailed in the Installation section below
and the `INSTALL` file.

A developer who wishes to modify `fswatch` should get the sources (either from a
source tarball or cloning the repository) and have the GNU Build System
installed on their machine.  Please read `README.gnu-build-system` to get further
details about how to bootstrap `fswatch` from sources on your machine.

Getting a copy of the source repository is not recommended unless you are a
developer, you have the GNU Build System installed on your machine, and you know
how to bootstrap it on the sources.

[release]: https://github.com/emcrisostomo/fswatch/releases

Installation
------------

See the `INSTALL` file for detailed information about how to configure and
install `fswatch`.  Since the `fswatch` builds and uses dynamic libraries, in
some platforms you may need to perform additional tasks before you can use
`fswatch`:

  * Make sure the installation directory of dynamic libraries (`$PREFIX/lib`) is
    included in the lookup paths of the dynamic linker of your operating system.
    The default path, `/usr/local/lib`, will work in nearly every operating
    system.
  * Refreshing the links and cache to the dynamic libraries may be required.  In
    GNU/Linux systems you may need to run `ldconfig`:

        $ ldconfig

`fswatch` is a C++ program and a C++ compiler compliant with the C++11 standard
is required to compile it.  Check your OS documentation for information about
how to install the C++ toolchain and the C++ runtime.

No other software packages or dependencies are required to configure and install
`fswatch` but the aforementioned APIs used by the file system monitors.

Documentation
-------------

`fswatch` provides the following [documentation]:

  * Texinfo documentation, included with the distribution.
  * HTML documentation.
  * PDF documentation.
  * A [wiki] page.
  * A man page.

`fswatch` official documentation is provided in Texinfo format.  This is the
most comprehensive source of information about `fswatch` and the only
authoritative one.  The man page, in particular, is a stub that suggests the
user to use the info page instead.

If you are installing `fswatch` using a package manager and you would like the
PDF manual to be bundled into the package, please send a feature request to the
package maintainer.

[documentation]: http://emcrisostomo.github.io/fswatch/doc
[wiki]: https://github.com/emcrisostomo/fswatch/wiki

Localization
------------

`fswatch` is localizable and internally uses GNU `gettext` to decouple
localizable string from their translation.  The currently available locales are:

  * English (`en`).
  * Italian (`it`).
  * Spanish (`es`).

To build `fswatch` with localization support, you need to have `gettext`
installed on your system.  If `configure` cannot find `<libintl.h>` or the
linker cannot find `libintl`, then you may need to manually provide their
location to `configure`, usually using the `CPPFLAGS` and the `LDFLAGS`
variables.  See `README.osx` for an example.

If `gettext` is not available on your system, `fswatch` shall build correctly,
but it will lack localization support and the only available locale will be
English.

Usage
-----

`fswatch` accepts a list of paths for which change events should be received:

    $ fswatch [options] ... path-0 ... path-n

The event stream is created even if any of the paths do not exist yet.  If they
are created after `fswatch` is launched, change events will be properly
received.  Depending on the watcher being used, newly created paths will be
monitored after the amount of configured latency has elapsed.

The output of `fswatch` can be piped to other program in order to process it
further:

    $ fswatch -0 path | while read -d "" event \
      do \
        // do something with ${event}
      done

To run a command when a set of change events is printed to standard output but
no event details are required, then the following command can be used:

    $ fswatch -o path | xargs -n1 -I{} program

The behaviour is consistent with earlier versions of `fswatch` (v. 0.x).
Please, read the _Compatibility Issues with fswatch v. 0.x_ section for further
information.

By default `fswatch` chooses the best monitor available on the current platform,
in terms of performance and resource consumption.  If the user wishes to specify
a different monitor, the `-m` option can be used to specify the monitor by name:

    $ fswatch -m kqueue_monitor path

The list of available monitors can be obtained with the `-h` option.

For more information, refer to the `fswatch` documentation.

Contributing
------------

Everybody is welcome to contribute to `fswatch`.  Please, see
[`CONTRIBUTING`][contrib] for further information.

[contrib]: CONTRIBUTING.md

Bug Reports
-----------

Bug reports can be sent directly to the authors.

Contact the Authors
-------------------

The author can be contacted on IRC, using the Freenode `#fswatch` channel.

-----

Copyright (c) 2013-2018 Enrico M. Crisostomo

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 3, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program.  If not, see <http://www.gnu.org/licenses/>.
