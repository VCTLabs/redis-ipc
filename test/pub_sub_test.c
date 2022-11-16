#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "redis_ipc.h"

void spawn_listener_process(void)
{
  json_object *message;
  pid_t pid = fork();
  int i;

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
  redis_ipc_subscribe_setting_notifications();

  for (i = 0; i < 7; i++)
  {
      message = redis_ipc_get_message_blocking();
      json_object_put(message);
  }

  struct timeval timeout = {2, 0};
  fprintf(stderr, "** This wait for message should time out in 2 sec...\n");
  message = redis_ipc_get_message_timeout(timeout);

  redis_ipc_unsubscribe_events();
  redis_ipc_unsubscribe_debug();
  redis_ipc_unsubscribe_setting_notifications();

  redis_ipc_cleanup(getpid());

  _exit(0);
}

int main(int argc, char **argv)
{
  json_object *event = NULL;
  json_object *setting = NULL;
  int child_status = -1;

  spawn_listener_process();

  redis_ipc_init("printer", "monitor");
  redis_ipc_send_debug(1, "printer starting to smoke");
  sleep(1);
  redis_ipc_send_debug(0, "printer on fire!!");
  sleep(1);

  event = json_object_new_object();
  json_object_object_add(event, "severity",
                         json_object_new_string("warning"));
  json_object_object_add(event, "message",
                         json_object_new_string("printer is down for the count"));
  redis_ipc_send_event("state", event);
  json_object_put(event);
  sleep(1);

  event = json_object_new_object();
  json_object_object_add(event, "severity",
                         json_object_new_string("alert"));
  json_object_object_add(event, "message",
                         json_object_new_string("there went our expensive paper"));
  json_object_object_add(event, "pages_remaining", json_object_new_int(0));
  redis_ipc_send_event("media", event);
  json_object_put(event);
  sleep(1);

  event = json_object_new_object();
  json_object_object_add(event, "severity",
                         json_object_new_string("info"));
  json_object_object_add(event, "message",
                         json_object_new_string("save trees, go digital"));
  redis_ipc_send_event(NULL, event);
  json_object_put(event);
  sleep(1);

  // try to generate notifications for settings changes in web component
  redis_ipc_config_settings_writer(RIPC_COMPONENT_ANY);
  fprintf(stderr, "** This full setting write should generate 'hset' message...\n");
  setting = json_object_new_object();
  json_object_object_add(setting, "colorspace",
                         json_object_new_string("purple and more purple"));
  json_object_object_add(setting, "theme",
                         json_object_new_string("boisterous"));
  redis_ipc_write_setting("web", setting);
  json_object_put(setting);
  sleep(1);

  fprintf(stderr, "** This single setting field write should generate 'hset' message...\n");
  redis_ipc_write_setting_field("web", "theme", "stealth");
  redis_ipc_config_settings_writer(RIPC_DEFAULT_SETTINGS_WRITER);

  redis_ipc_cleanup(getpid());
  wait(&child_status);

  return 0;
}
