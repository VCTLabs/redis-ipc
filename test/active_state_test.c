#include <stdio.h>
#include <unistd.h>
#include "redis_ipc.h"

int main(int argc, char **argv)
{
  char *write_state = NULL, *read_state = NULL;

  redis_ipc_init("blackbox", "main");

  write_state = "SQUEAKING";
  redis_ipc_write_active_state(write_state);
  read_state = redis_ipc_read_active_state("blackbox");
  printf("wrote '%s', got back '%s'\n", write_state, read_state);
  free(read_state);

  write_state = "BOUNCING";
  redis_ipc_write_active_state(write_state);
  read_state = redis_ipc_read_active_state("blackbox");
  printf("wrote '%s', got back '%s'\n", write_state, read_state);
  free(read_state);

  write_state = "SPLASHING";
  redis_ipc_write_active_state(write_state);
  read_state = redis_ipc_read_active_state("blackbox");
  printf("wrote '%s', got back '%s'\n", write_state, read_state);
  free(read_state);

  read_state = redis_ipc_read_active_state("purplebox");
  if (read_state == NULL)
  {
    printf("read NULL for non-existent state\n");
  }
  else
  {
    printf("read non-NULL '%s' for non-existent state??\n", read_state);
    free(read_state);
  }

  redis_ipc_cleanup(getpid());

  return 0;
}
