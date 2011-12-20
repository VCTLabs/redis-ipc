#include <unistd.h>
#include "redis_ipc.h"

int main(int argc, char **argv)
{
  json_object *setting = NULL;
  json_object *status = NULL;
  
  redis_ipc_init("session", "main");

  status = json_object_new_object();
  json_object_object_add(status, "open",
                         json_object_new_string("until closed"));
  json_object_object_add(status, "procedure",
                         json_object_new_string("complicated"));
  redis_ipc_write_status(status);
  json_object_put(status);

  status = redis_ipc_read_status("session");
  json_object_put(status);

  redis_ipc_cleanup(getpid());

  return 42;
}
