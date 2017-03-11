examples
========

The following are currently more "ultra-simple examples" than tests,
in that they mostly don't catch errors to return failures. 

In order to observe these examples working, you need to have a redis server 
configured with a unix socket (see toplevel README).

multithread_test.c
  demo debug messages from two different threads, 
  can use 'redis-cli -s /tmp/redis-ipc/socket monitor' to view them

command_result_test.c : 
  demo command queues

pub_sub_test.c : 
  demo events (including "debug" events with priority level)

settings_status_test.c : 
  demo settings and status (status is like settings but only owner component
  can write them)
