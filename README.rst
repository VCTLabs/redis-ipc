redis-ipc 
=========

redis-ipc is an example of how redis_ can be used as an advanced IPC 
mechanism on an embedded Linux system, for instance as a substitute for the
more common choice of dbus. 

redis-ipc is intended to make communication among different logical components
of a system convenient. It is not intended to replace shared memory for high 
data-rate transfers between processes, where lowest possible overhead is key,
but to provide a convenient and reliable way to implement the following
IPC mechanisms:

* command queues 
* settings 
* status 
* event channels

"But, but... redis for *embedded* applications??"

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

After covering the many attractions of redis_, it is only fair to point out an
important limitation: the `lack of security features`_ (toy authentication and no ability to restrict
capabilities of connected clients) makes it highly unsuitable for access by 
untrusted users. Security-wise (and performance-wise, for that matter) it is better 
to use unix sockets than a locally-bound tcp socket, so that filesystem permissions can be 
used to restrict socket access to a certain user or group. However always keep in mind that 
a rogue process running as that authorized user or group gains full admin powers over the server, 
including snooping of all redis_ activity and making runtime changes to the config.

For that reason, **never** use redis_ in security-sensitive environments unless 
there are solid external mechanisms for restricting access (sandboxing, 
custom SELinux policy limiting redis connections to specific
trusted applications), and for security-critical tasks the principle of 
layered defense calls for a more secure store as an additional line of defense 
-- credit card info cached in an unencrypted redis store would be
such a juicy target for any attackers who made it onto the server!

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
build and install it on your Linux development box.

* Install build dependencies

  * C/C++ toolchain
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

* Run the compile

  * native build::

      # also builds the library, in addition to some simple example apps
      make testprogs 

  * cross-compile build::

      # also builds the library, in addition to some simple example apps
      make testprogs CROSS_COMPILE=<toolchain prefix> SYSROOT=<cross-compile staging area>

    * **CROSS_COMPILE** is everything up to (and including) the last '-' in the tool names,
      e.g. if the C compiler is arm-none-linux-gnueabi-gcc then
      
        **CROSS_COMPILE=arm-none-linux-gnueabi-**

    * **SYSROOT** is the base path of your staging area that has cross-compiled versions of the
      dependency libraries, e.g. if the cross-compiled hiredis library is under 
      /home/sjl/yocto/build/tmp/sysroots/armv5te-poky-linux-gnueabi/usr/lib
      then
      
        **SYSROOT=/home/sjl/yocto/build/tmp/sysroots/armv5te-poky-linux-gnueabi/**

Running redis-ipc
=================

After building redis-ipc for the desired platform, try running it against a redis server.
The redis server needs to be configured to use a unix socket, the path of which is
currently hard-coded to /tmp/redis-ipc/socket in this library

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

When redis-ipc is no longer neaded, there is a corresponding function to free 
resources ::

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
  // while printing or else an in-progress job can't be cancelled

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
