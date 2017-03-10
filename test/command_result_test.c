#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "redis_ipc.h"

void spawn_command_handler_process(void)
{
  json_object *command = NULL, *result = NULL;
  pid_t pid = fork();

  // parent waits for child to start listening, then returns
  if (pid > 0) 
  {
    sleep(1);
    return;
  }

  // child continues on
  redis_ipc_init("streaming", "recorder");

  command = redis_ipc_receive_command_blocking("video", 0);

  result = json_object_new_object();
  json_object_object_add(result, "code",
                         json_object_new_int(-2));
  json_object_object_add(result, "message",
                         json_object_new_string("Next time, say 'please'"));
  redis_ipc_send_result(command, result);
  json_object_put(command);
  json_object_put(result);

  redis_ipc_cleanup(getpid());

  _exit(0);
}

int main(int argc, char **argv)
{
  json_object *command = NULL, *result = NULL;
  int timeout = 10;
  int child_status = -1;

  spawn_command_handler_process();
  
  redis_ipc_init("web", "requestor");

  command = json_object_new_object();
  json_object_object_add(command, "method",
                         json_object_new_string("control_recording"));
  json_object_object_add(command, "params",
                         json_object_new_string("start"));
  result = redis_ipc_send_command_blocking("streaming", "video", command, timeout);
  json_object_put(command);

  if (result != NULL)
  {
    printf("Received result: %s\n", json_object_to_json_string(result));
    json_object_put(result);
  }
  else
  {
    printf("Timed out waiting for result\n");
  }

  redis_ipc_cleanup(getpid());
  wait(&child_status);

  return 42;
}
