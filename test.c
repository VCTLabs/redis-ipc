#include "redis_ipc.h"

int main(int argc, char **argv)
{
  json_object *event = NULL;
  
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

  return 42;
}
