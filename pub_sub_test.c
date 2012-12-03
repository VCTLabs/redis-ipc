#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "redis_ipc.h"

void spawn_listener_process(void)
{
  json_object *message;
  pid_t pid = fork();

  // parent waits for child to start listening, then returns
  if (pid > 0) 
  {
    sleep(1);
    return;
  }

  // child continues on
  redis_ipc_init("web", "listener");

  redis_ipc_subscribe_events("printer", NULL);
  redis_ipc_subscribe_debug("printer");

  message = redis_ipc_get_message_blocking();
  json_object_put(message);

  message = redis_ipc_get_message_blocking();
  json_object_put(message);

  redis_ipc_unsubscribe_events();
  redis_ipc_unsubscribe_debug();

  redis_ipc_cleanup(getpid());

  _exit(0);
}

int main(int argc, char **argv)
{
  json_object *event = NULL;
  int child_status = -1;

  spawn_listener_process();
  
  redis_ipc_init("printer", "monitor");
  redis_ipc_send_debug(1, "printer starting to smoke");
  redis_ipc_send_debug(0, "printer on fire!!");

  event = json_object_new_object();
  json_object_object_add(event, "severity",
                         json_object_new_string("warning"));
  json_object_object_add(event, "message",
                         json_object_new_string("printer is down for the count"));
  redis_ipc_send_event("state", event);
  json_object_put(event);

  event = json_object_new_object();
  json_object_object_add(event, "severity",
                         json_object_new_string("alert"));
  json_object_object_add(event, "message",
                         json_object_new_string("there went our expensive paper"));
  json_object_object_add(event, "pages_remaining", json_object_new_int(0));
  redis_ipc_send_event("media", event);
  json_object_put(event);

  event = json_object_new_object();
  json_object_object_add(event, "severity",
                         json_object_new_string("info"));
  json_object_object_add(event, "message",
                         json_object_new_string("save trees, go digital"));
  redis_ipc_send_event(NULL, event);
  json_object_put(event);

  redis_ipc_cleanup(getpid());
  wait(&child_status);

  return 42;
}
