===========
 redis-ipc
===========

redis-ipc is an example of how redis_ can be used as an advanced IPC
mechanism on an embedded Linux system, for instance as a substitute for
the more common choice of dbus.

|ci| |conda-dev| |codeql| |cpplint| |coverage|

|pre| |cov|

|tag| |license| |std|

redis-ipc is intended to make communication among different logical
components of a system convenient. It is not intended to replace shared
memory for high data-rate transfers between processes, where lowest
possible overhead is key, but to provide a convenient and reliable way
to implement the following IPC mechanisms:

* command queues
* settings
* status
* event channels

"But, but... redis for *embedded* applications??"


Quick Start Package Install
===========================

redis-ipc comes in 2 flavors, a python class module and a lightweight C
library implementation (this repo). The python module has moved to a
`new home`_.

.. _new home: https://github.com/VCTLabs/redis-ipc-py

Packages are available for Ubuntu_, and the latest can be installed on
Gentoo using the ebuilds in `this portage overlay`_. To build from
source, see `Building redis-ipc`_ below.


.. _Ubuntu: https://launchpad.net/~nerdboy/+archive/ubuntu/embedded
.. _this portage overlay: https://github.com/VCTLabs/embedded-overlay/


Making Changes & Contributing
=============================

This repo is now pre-commit_ enabled for various linting and format
checks.  The checks run automatically on commit and will fail the
commit (if not clean) with some checks performing simple file corrections.

If other checks fail on commit, the failure display should explain the error
types and line numbers. Note you must fix any fatal errors for the
commit to succeed; some errors should be fixed automatically (use
``git status`` and `` git diff`` to review any changes).

See the pre-commit docs under ``docs/dev/`` for more information:

* pre-commit-config_
* pre-commit-usage_

You will need to install pre-commit before contributing any changes;
installing it using your system's package manager is recommended,
otherwise install with pip into your usual virtual environment using
something like::

  $ sudo emerge pre-commit  --or--
  $ pip install pre-commit

then install it into the repo you just cloned::

  $ git clone https://github.com/VCTLabs/redis-ipc
  $ cd redis-ipc/
  $ pre-commit install

It's usually a good idea to update the hooks to the latest version::

    pre-commit autoupdate


.. _pre-commit: http://pre-commit.com/
.. _cpplint: https://github.com/cpplint/cpplint
.. _pre-commit-config: docs/dev/pre-commit-config.rst
.. _pre-commit-usage: docs/dev/pre-commit-usage.rst


Prerequisites
-------------

A supported linux distribution, mainly something that uses either
``.ebuilds`` (eg, Gentoo or funtoo) or ``.deb`` packages, starting with at
least Ubuntu bionic or Debian stretch (see the above PPA package repo
on Launchpad).

Make sure you have the ``add-apt-repository`` command installed and
then add the PPA:

::

  $ sudo apt-get install software-properties-common
  $ sudo add-apt-repository -y -s ppa:nerdboy/embedded
  $ sudo apt-get install libredis-ipc-dev redis-tools redis-server


.. note:: Since the package series currently published are for bionic/focal,
          building from source is recommended if installing on Debian.


If you get a key error you will also need to manually import the PPA
signing key like so:

::

  $ sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys <PPA_KEY>

where <PPA_KEY> is the key shown in the launchpad PPA page under "Adding
this PPA to your system", eg, ``41113ed57774ed19`` for `Embedded device ppa`_.


.. _Embedded device ppa: https://launchpad.net/~nerdboy/+archive/ubuntu/embedded


Build and test with Tox
-----------------------

As long as you have git and at least Python 3.6, then the "easy" dev
install is to clone this repository and install `tox`_.

After cloning the repository, you can run the current tests using either
cmake or autotools with the ``tox`` command.  It will build a virtual
python environment with most of the build dependencies (except the shared
libraries above and the autotools bits) and then run the tests. For cmake
plus test coverage, you would first install your toolchain, the required
json-c and hiredis libraries, redis-server, and tox, then run the following
commands:

::

  $ git clone https://github.com/VCTLabs/redis-ipc
  $ cd redis-ipc
  $ tox -e ctest

The above will start the redis server, build and run the tests with coverage
display, and stop the redis server. Alternatively, you can manipulate the
server manually and use ``tox -e tests`` instead.  To run the autotools
build, you may need most or all of the following packages installed; for
example on Ubuntu you might need:

* build-essential, make, libjson-c-dev, libhiredis-dev, redis-server
  (from above)
* autoconf, autoconf-archive, automake, pkg-config, libtool

.. note:: The default libjson-c version (0.13) on Ubuntu focal is broken,
  so you should add the PPA and install the 0.15 package instead.

There are several ``tox -e`` environment commands available:

* ``ctest`` - build/run tests using ctest (**with** redis-server handling)
* ``tests`` - build/run tests using cmake (**without** redis-server handling)
* ``bionic`` - build/run tests using cmake (**with** redis-server handling)
* ``grind`` - build/run using cmake and valgrind (**with** redis-server handling)
* ``clean`` - clean the cmake build/ directory/files
* ``auto`` - build/run tests using autotools (**with** redis-server handling)
* ``autoclean`` - clean all the autotools cruft
* ``lint`` - run the cpplint style checks

With the additional dependencies of LLVM/Clang >= 12 you can try the LLVM
source-based coverage alternative to gcov/lcov-based coverage.  First install
the above toolchain, then run the following::

  $ CC=clang CXX=clang++ tox -e clang

If you installed a newer version than 12, eg, 13, then prepend the version
using ``ENV_LLVM_VER=13`` to the above command.

.. note:: Without the PPA, Bionic has an older GTest package and needs an
          extra cmake arg.

See the `Github workflow files`_ for more details on the packages installed
for each runner OS environment.


.. _tox: https://github.com/tox-dev/tox
.. _Github workflow files: https://github.com/VCTLabs/redis-ipc/tree/develop/.github/workflows


Quick Start Dev Environment
===========================

Packages should eventually be available in `Conda Forge`_ but you can
always use Conda's ``devenv`` support to build/install locally inside a
Conda environment. This is the recommended method if you can't use the
PPA or Gentoo overlay. Set your default shell to ``bash`` if not already
set.

Prerequisites
-------------

Install either Anaconda_ or Miniconda_ (we recommend miniconda) and add
the ``conda-forge`` channel, then install the ``conda-devenv`` package.

* Download the miniconda linux-64 installer and run it
* Let the installer add the conda init bits to your ``.bashrc``
* Source your shell environment: ``source ~/.bashrc``
* Install ``conda-devenv``::

    conda config --append channels conda-forge
    conda install -n base conda-devenv

* Clone this repository::

    git clone https://github.com/VCTLabs/redis-ipc.git

* Create a new conda devenv environment::

    cd redis-ipc/
    conda devenv

This command will create the conda environment called ``redis-ipc-test``,
which can take a few minutes to complete the first time. This will install
the conda toolchain and all required dependencies to build from source
(see the contents of the ``environment.devenv.yml`` file for details).

* Activate the environment::

    conda activate redis-ipc-test

Now you can use the usual ``cmake`` configure and build steps (see the
`Cmake build`_ section below) or you can run the following one-liner
for a quick build-and-test::

  ctest --build-config RelWithDebInfo --build-generator "Ninja" \
    --build-and-test . build --build-options -DRIPC_DISABLE_SOCK_TESTS=1 \
    -DCMAKE_INSTALL_PREFIX=$CONDA_PREFIX -DCMAKE_INSTALL_LIBDIR=lib \
    --test-command ctest -V --build-config RelWithDebInfo

.. note:: The above command will omit running the socket tests, but if
    already have a running ``redis`` server available, you can set the
    ``RIPC_DISABLE_SOCK_TESTS`` argument to ``0`` instead.

Whenever the above dependencies change or you alter your local conda
environment, rebuild your ``devenv``::

    conda devenv

When finished, deactivate the environment::

    conda deactivate


.. _Conda Forge: https://anaconda.org/conda-forge/repo
.. _Anaconda: https://www.anaconda.com/download
.. _Miniconda: https://conda.io/miniconda.html


Digression into the wonders of redis
====================================

redis_ is a powerful communications protocol that has been undeservedly
typecast, in the entertainment industry sense, as a backend service (database
replacement / database caching layer) for web applications. Many non-web
developers have not even heard of redis, and if they have, it was probably in
the context of peeking at some no-sql discussions to see what those crazy
big-server/cloud web-service developers are up to nowadays -- without the
realization that they have just stumbled across a general-purpose technology
that could likely be put to good use in one of their own non-webcentric
projects someday.

Many of the features that make redis_ appealing for web applications are still
relevant for IPC among local applications:

* `low overhead`_ -- maxes out at thousands of operations per second on
  a 1GHz ARM and 10s of thousands of operations per second on a desktop-class CPU,
  so system load is low for practical usage (even hundreds of operations per second)
* wide variety of `language bindings`_ -- name your favorite non-toy language,
  and there is a good chance the redis binding already exists
* `convenient monitoring for troubleshooting`_ -- snoop all the incoming commands as a
  human-readable stream of text, rather than needing specialized debugging tools to decode
  binary messages in order to make *any* sense of them (and having to update said tools
  every time a new custom message is defined)
* atomic operations -- along with the ability to turn on monitoring, can make developing
  multi-threaded and multi-process applications so much easier

and finally one more which is specifically relevant for embedded software:

* portable -- cross-compiles nicely, available as `openembedded recipe`_


Important caveat regarding redis security
=========================================

After covering the many attractions of redis_, it is only fair to point
out an important limitation: the `lack of security features`_ (toy
authentication and no ability to restrict capabilities of connected
clients) makes it highly unsuitable for access by untrusted users.

Security-wise (and performance-wise, for that matter) it is better to
use unix sockets than a locally-bound tcp socket, so that filesystem
permissions can be used to restrict socket access to a certain user or
group. However always keep in mind that a rogue process running as that
authorized user or group gains full admin powers over the server,
including snooping of all redis_ activity and making runtime changes to
the config.

For that reason, **never** use redis_ in security-sensitive environments
unless there are solid external mechanisms for restricting  access (sandboxing,
custom SELinux policy limiting redis connections to specific trusted
applications), and for security-critical tasks the  principle of layered
defense calls for a more secure store as an additional line of defense,
eg, credit card info cached in an unencrypted redis store would be such
a juicy target for any attackers who made it onto the server!

Example of sensible scenarios for redis deployment:

* Use redis to coordinate processes in regression test framework for assessing
  current development status of an embedded device. Connections are limited to
  localhost, and the only other users with accounts on the workstation are
  trusted fellow teammates on the project.

* Use redis to maintain settings and status for an embedded device. Connections
  are again limited to localhost, and in normal operations (i.e. not development mode)
  there are NO network logins enabled to the device.


Building redis-ipc
==================

Now back to the star of this show, namely redis-ipc, starting with how to
build and install it on your Linux development box (redis-ipc now supports
both autotools and CMake build systems, so in the following steps choose
one or the other).

* Install build dependencies

  * C/C++ toolchain
  * pkg-config
  * make
  * cmake --or-- automake/autoconf/libtool
  * hiredis_
  * json-c_

  On a development system with Debian/Ubuntu/Mint, this generally amounts to::

    apt-get install libhiredis-dev libjson0-dev

  with Gentoo::

    emerge dev-libs/hiredis dev-libs/json-c

  with Fedora (or CentOS/RHEL using EPEL_)::

    yum install hiredis-devel json-c-devel

* Check out redis-ipc source code (no tarball releases yet)::

    git clone https://github.com/VCTLabs/redis-ipc.git
    cd redis-ipc/


CMake build
-----------

The cmake tools can be run in several ways, and follow the standard set
of (cmake) out-of-tree build steps.

* Create the build directory::

    mkdir build && cd build/

* Configure the build::

    cmake -DCMAKE_BUILD_TYPE=Debug ..

* Build it::

    make

* Run the tests::

    make check


Autotools build
---------------

The autotools build will create the standard set of Makefiles and the
``configure`` script.

* Generate and run configure::

    ./autogen.sh && ./configure

* Run the compile

  * native build::

      # also builds the library, in addition to some simple example apps
      make

  * cross-compile build::

      # also builds the library, in addition to some simple example apps
      make CROSS_COMPILE=<toolchain prefix> SYSROOT=<cross-compile staging area>

    * **CROSS_COMPILE** is everything up to (and including) the last '-' in the
      tool names, e.g. if the C compiler is arm-none-linux-gnueabi-gcc then

        **CROSS_COMPILE=arm-none-linux-gnueabi-**

    * **SYSROOT** is the base path of your staging area that has cross-compiled
      versions of the dependency libraries, e.g. if the cross-compiled hiredis
      library is under

        /home/sjl/yocto/build/tmp/sysroots/armv5te-poky-linux-gnueabi/usr/lib

      then

        **SYSROOT=/home/sjl/yocto/build/tmp/sysroots/armv5te-poky-linux-gnueabi/**

Running redis-ipc
=================

After building redis-ipc for the desired platform, try running it against a redis server.
The redis server needs to be configured to use a unix socket, the path of which
defaults to $RPC_RUNTIME_DIR/socket, where RPC_RUNTIME_DIR defaults to /tmp/redis-ipc
but may be overridden at compile time. The socket path may also be overridden at
runtime with the environment variable ``RIPC_SERVER_PATH``.

redis.conf excerpt::

  # Accept connections on the specified port, default is 6379.
  # If port 0 is specified Redis will not listen on a TCP socket.
  port 0

  # Specify the path for the unix socket that will be used to listen for
  # incoming connections. There is no default, so Redis will not listen
  # on a unix socket when not specified.

  unixsocket /tmp/redis-ipc/socket
  # this allows connections by the user who starts the server (or by root, of course)
  unixsocketperm 500
  # alternatively, allow connections from the group under which server was started
  #unixsocketperm 550

Next verify the combination of redis-server and redis-ipc library are working together
by running the simple example programs, which each end in *_test*. Use LD_LIBRARY_PATH
to run the programs from the build directory (or, if cross-compiling, a copy of build
directory loaded on the embedded target) prior to a system-wide install ::

  LD_LIBRARY_PATH=. ./command_result_test

Expected output for each example app has been provided in files ending in *_test.out*.

Developing with redis-ipc
=========================

As mentioned in the intro, redis-ipc implements the following mechanisms:

* command queues
* settings
* status
* event channels

Command queues are a method for any component to request an action from
another component, and receive a result after the command has been processed.
Each component that exports actions to other components would own one or
more command queues. When sending a command, the queue is specified by
component and "subqueue" to allow components to manage multiple queues
that are processed with different priorities.

Settings are hashes representing the current configuration of each component.
The settings for a single component can all be read atomically and written
atomically, to avoid bugs where one component gets into an inconsistent state
by reading settings when partially updates by another component. Note that
settings changes across multiple components are *not* atomic, so complicated
designs where settings consistency depends on updating multiple components at
the same time would need to implement that separately, e.g. with some form of
locking.

Status are also hashes, but represent a component's current runtime state
instead of representing how a component has been configured. While settings
are likely written by a single component, each component maintains its own
status with any state info that is of interest to one or more other components.

Event channels are an efficient way to broadcast events from one component to
any others that might be interested (i.e. "subscribers). At the toplevel,
event channels are grouped into normal channels and debug channels that are
accessed by a separate set of calls. This segregation of normal events from
debug messages makes it obvious in the code which messages are only meant for
debugging the component, and makes it easy to log/observe detailed debug info
while normal subscribers can listen to normal events without having to discard
a flood of debug events when debugging is enabled (by runtime configuration or
special debug compile).

Most data handled by redis-ipc (commands, command replies, settings, status,
and events) is formatted into `JSON objects`_, meaning associative arrays
containing key/value pairs. The only exception is that
an individual field within a setting or status object can be accessed as
a cstring. json-c library is used as the JSON implementation. Actually,
debug events are another exception, being specified with a numeric priority
level and a message with printf-style format + arguments.

As typical for a C library dealing with dynamically created objects, reference
counting is used to ensure memory is released at the proper time. redis-ipc
returns new JSON objects with one reference that the caller is responsible for
freeing with json_object_put(). C++ applications can make use of the json.hh
wrapper supplied in redis-ipc that takes and drops references on the underlying
json-c json_object when appropriate ::

  #include "json.hh"

  ...

  void monitor_printer()
  {
    redis_ipc_subscribe_events("printer", NULL);
    // does not take a new reference on json_object being wrapped
    // because redis_ipc_get_message_blocking() already took one
    json next_printer_event(redis_ipc_get_message_blocking());
    cout << "Event priority:" << next_printer_event.get_field("priority");
  }
  // reference to  json_object dropped when next_printer_event goes out of scope

**Common API**

Every thread and process using redis-ipc must individually call the
init function prior to any of the other calls ::

  int redis_ipc_init(const char *this_component, const char *this_thread);

Example::

  // monitor process (or thread) of printer software component
  redis_ipc_init("printer", "monitor");

When redis-ipc is no longer needed, there is a corresponding function to
free resources ::

  int redis_ipc_cleanup(pid_t tid);

Examples::

  // single process closing down
  redis_ipc_cleanup(getpid());

  // one thread of multi-thread process closing down
  // see gettid() definition in redis_ipc.c if your libc lacks it
  redis_ipc_cleanup(gettid());

**Command queue API**

Command queues currently have a blocking implementation.

The JSON object for a command automatically gets 2 attributes added
as a part of submission

* command_id : unique ID for command, including component name and thread id
  of the submitter
* results_queue : name of queue on which the result object should be pushed
  when command has been processes, also based on component name and thread id
  (each thread submitting commands has its own queue to wait on)

The JSON object for reporting back a command result to the submitter
automatically gets the command_id added, to ensure commands and their
results can be reliably associated.

**Important note**: To avoid memory leaks, callers of command queue API must
drop references to command objects and result objects when finished with them.

Processes/threads that execute commands block until a command arrives ::

  json_object * redis_ipc_receive_command_blocking(const char *subqueue,
                                              unsigned int timeout);

then when another process/thread submits a command, it will block until the
command has been completed (or timeout for waiting has expired) ::

  json_object * redis_ipc_send_command_blocking(const char *dest_component,
                                              const char *subqueue,
                                              json_object *command,
                                              unsigned int timeout);

which happens when the executing process/thread reports back the command
results with ::

  int redis_ipc_send_result(const json_object *completed_command, json_object *result);

Example::

  // printer component has 2 queues, "print" and "cancel"
  // because cancel commands need a separate queue that is checked even
  // while printing or else an in-progress job can't be canceled

  // non-printer component requests printing of file
  json_object *command = json_object_new_object();
  json_object_object_add(command, "pagesize",
                         json_object_new_string("A4"));
  json_object_object_add(command, "file",
                         json_object_new_string("/tmp/job1231.pdf"));
  json_object *result = redis_ipc_send_command_blocking("printer", "print", command, timeout);
  json_object *job_id_obj = json_object_object_get(result, "job-id");
  char *job_id_str = json_object_get_string(job_id_obj);
  json_object_put(command);
  json_object_put(result);
  json_object_put(job_id_obj);

  // non-printer component later decides to cancel print job
  command = json_object_new_object();
  json_object_object_add(command, "job-id",
                         json_object_new_string(job_id_str));
  json_object *result = redis_ipc_send_command_blocking("printer", "cancel", command, timeout);
  json_object_put(command);
  json_object_put(result);


**Settings API**

Multiple settings for a single component can be updated atomically
as multiple key/value pairs in a JSON object ::

  int redis_ipc_write_setting(const char *owner_component, const json_object *fields);

or a single setting can be updated by name, with both name and value supplied
as strings ::

  int redis_ipc_write_setting_field(const char *owner_component, const char *field_name,
                                    const char *field_value);

Similarly, all settings belonging to a single component can be read as
JSON object containing key/value pairs ::

  json_object * redis_ipc_read_setting(const char *owner_component);

or a single setting can be requested by name, with both name and returned value
as strings ::

  char * redis_ipc_read_setting_field(const char *owner_component, const char *field_name);

**Status API**

Multiple status for a single component can be updated atomically
as multiple key/value pairs in a JSON object ::

  int redis_ipc_write_status(const json_object *fields);

or a single status can be updated by name, with both name and value supplied
as strings ::

  int redis_ipc_write_status_field(const char *field_name, const char *field_value);

Similarly, all settings belonging to a single component can be read as
JSON object containing key/value pairs ::

  json_object * redis_ipc_read_status(const char *owner_component);

or a single setting can be requested by name, with both name and returned value
as strings ::

  char * redis_ipc_read_status_field(const char *owner_component, const char *field_name);

**Event API**

Event channels currently have a blocking implementation for event listeners.

Channels for normal events are grouped according to component that
generates the event. When a component sends a normal message it must also
supply a "subchannel" as the most specific part of this addressing scheme, with
each subchannel hopefully given a meaningful name to indicate what sort of
messages subscribers should expect.

When a component sends a debug message, it supplies a debug level, so that the
debug channels can skip sending debug messages that are higher than the
currently configured debug verbosity (although, at the moment verbosity happens
to be hard-coded to the value 5, meaning everything 5 and under gets
broadcast...)

Listeners must sign up ahead of time to get the events of interest;
there is no backlog for catching up on events posted to a channel before
a listener subscribed. Event channels of interest are specified by
the component generating the events and a subchannel name, where subchannel
name may represent a topic that applies to multiple components.

**Important note**: To avoid memory leaks, callers of event API must drop
references to event objects when finished with them.

Listeners can subscribe to channels with normal events ::

  int redis_ipc_subscribe_events(const char *component, const char *subchannel)

and/or channels with debug events ::

  int redis_ipc_subscribe_debug(const char *component);

Examples::

  // subscribe to all printer-related events
  redis_ipc_subscribe_events("printer", NULL);

  // subscribe to all warnings that should be displayed to user
  redis_ipc_subscribe_events(NULL, "warnings");

  // subscribe specifically to warnings from printer component
  redis_ipc_subscribe_events("printer", "warnings");

  // subscribe to debug messages from printer component
  redis_ipc_subscribe_debug("printer");

A component generates a normal event with one or more named attributes
contained in a JSON object, and broadcasts it on one of its subchannels ::

  int redis_ipc_send_event(const char *subchannel, json_object *message)

Example::

  // printer component sends a low-on-paper event to its warning subchannel
  json_object *event = json_object_new_object();
  json_object_object_add(event, "severity",
                         json_object_new_string("2"));
  json_object_object_add(event, "type",
                         json_object_new_string("LOW-ON-PAPER"));
  redis_ipc_send_event("warnings", event);


or broadcasts a debug event with a debug level and printf-formatted message
that then get turned into a JSON object ::

  int redis_ipc_send_debug(unsigned int debug_level, const char *format, ...)

Example::

  // completely hypothetical example, ahem...
  redis_ipc_send_debug(RIPC_DBG_ERROR, "redis_ipc_send_command_blocking(): invalid result");

Listening components can retrieve the next normal/debug event ::

  json_object * redis_ipc_get_message_blocking(void)

Example::

  json object *message = redis_ipc_get_message_blocking();
  // do stuff with message
  json_object_put(message);

Testing/troubleshooting with redis-ipc
======================================

One of the great features of using redis for system-wide IPC is the ability
to watch the interactions between components using the **monitor** command
from redis-cli utility. Another great use is in unit testing of a single
component, where a test script can push commands, update settings, check
status and so forth. For both reasons it is useful to understand how each
feature is implemented as redis data structures.

@@@TODO

Since redis-ipc requires the redis server to use a unix socket rather than tcp,
remember to specify the socket path when running redis-cli ::

  redis-cli -s /tmp/redis-ipc/socket

.. _redis: http://redis.io/
.. _low overhead: http://www.bango29.com/squeezing-cubieboard-for-performance/
.. _language bindings: http://redis.io/clients
.. _convenient monitoring for troubleshooting: http://redis.io/commands/MONITOR
.. _openembedded recipe: http://cgit.openembedded.org/cgit.cgi/meta-openembedded/tree/meta-oe/recipes-extended/redis/redis_2.6.9.bb?h=master
.. _lack of security features: http://redis.io/topics/security
.. _hiredis: https://github.com/redis/hiredis
.. _json-c: https://github.com/json-c/json-c/wiki
.. _EPEL: https://fedoraproject.org/wiki/EPEL
.. _JSON objects: https://en.wikipedia.org/wiki/Json

.. |ci| image:: https://github.com/VCTLabs/redis-ipc/actions/workflows/smoke.yml/badge.svg
    :target: https://github.com/VCTLabs/redis-ipc/actions/workflows/smoke.yml
    :alt: GitHub CI Smoke Test Status

.. |codeql| image:: https://github.com/VCTLabs/redis-ipc/actions/workflows/codeql.yml/badge.svg
    :target: https://github.com/VCTLabs/redis-ipc/actions/workflows/codeql.yml
    :alt: GitHub CI CodeQL Status

.. |conda-dev| image:: https://github.com/VCTLabs/redis-ipc/actions/workflows/conda-dev.yml/badge.svg
    :target: https://github.com/VCTLabs/redis-ipc/actions/workflows/conda-dev.yml
    :alt: GitHub CI Conda-dev Status

.. |cpplint| image:: https://github.com/VCTLabs/redis-ipc/actions/workflows/cpplint.yml/badge.svg
    :target: https://github.com/VCTLabs/redis-ipc/actions/workflows/cpplint.yml
    :alt: GitHub CI Cpplint Status

.. |coverage| image:: https://github.com/VCTLabs/redis-ipc/actions/workflows/coverage.yml/badge.svg
    :target: https://github.com/VCTLabs/redis-ipc/actions/workflows/coverage.yml
    :alt: Coverage workflow

.. |pre| image:: https://img.shields.io/badge/pre--commit-enabled-brightgreen?logo=pre-commit&logoColor=white
   :target: https://github.com/pre-commit/pre-commit
   :alt: pre-commit

.. |cov| image:: https://raw.githubusercontent.com/VCTLabs/redis-ipc/badges/develop/test-coverage.svg
    :target: https://github.com/VCTLabs/redis-ipc/
    :alt: Test coverage

.. |license| image:: https://badges.frapsoft.com/os/gpl/gpl.png?v=103
    :target: https://opensource.org/licenses/GPL-2.0/
    :alt: License

.. |tag| image:: https://img.shields.io/github/v/tag/VCTLabs/redis-ipc?color=green&include_prereleases&label=latest%20release
    :target: https://github.com/VCTLabs/redis-ipc/releases
    :alt: GitHub tag (latest SemVer, including pre-release)

.. |std| image:: https://img.shields.io/badge/Standards-C++11%20%20C99-00000.svg
    :target: https://isocpp.org/wiki/faq/cpp11
    :alt: Other standards
