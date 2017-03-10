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

  setting = json_object_new_object();
  json_object_object_add(setting, "auto_finalize",
                         json_object_new_string("no way"));
  // this should fail, component is not authorized to write settings
  // (not even its own)
  redis_ipc_write_setting("session", setting); 
  json_object_put(setting);
  redis_ipc_write_setting_field("session", "location", "right here"); 

  // should come back empty since above write failed
  setting = redis_ipc_read_setting("session");
  json_object_put(setting);
  redis_ipc_read_setting_field("session", "location");

  redis_ipc_cleanup(getpid());

  redis_ipc_init("db", "main");

  status = redis_ipc_read_status("session");
  json_object_put(status);

  setting = json_object_new_object();
  json_object_object_add(setting, "num_copies",
                         json_object_new_string("until ink runs out"));
  json_object_object_add(setting, "paper_type",
                         json_object_new_string("wrinkled"));
  redis_ipc_write_setting("printer", setting);
  json_object_put(setting);

  redis_ipc_write_setting_field("printer", "contrast", "none"); 

  setting = redis_ipc_read_setting("printer");
  redis_ipc_read_setting_field("printer", "paper_type"); 
  json_object_put(setting);

  redis_ipc_cleanup(getpid());

  return 42;
}
