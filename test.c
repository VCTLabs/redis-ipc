#include "redis_ipc.h"

int main(int argc, char **argv)
{
  
  redis_ipc_init("printer", "monitor");
  redis_ipc_send_debug(1, "printer starting to smoke");
  redis_ipc_send_debug(0, "printer on fire!!");

  return 42;
}
