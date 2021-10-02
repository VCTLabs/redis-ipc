#define _GNU_SOURCE // for gettid()
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <pthread.h>
#include "redis_ipc.h"

// gettid() is missing a libc wrapper before glibc 2.30
// (manpage even mentions it)
#if (__GLIBC_MINOR__ < 30)
pid_t gettid()
{
  return syscall(SYS_gettid);
}
#endif

void *run_printer_thread(void *data)
{
  redis_ipc_init("printer", "monitor");
  redis_ipc_send_debug(1, "printer starting to smoke");
  redis_ipc_send_debug(0, "printer on fire!!");
  redis_ipc_cleanup(gettid());

  return 0;
}

void *run_button_thread(void *data)
{
  redis_ipc_init("button", "watcher");
  redis_ipc_send_debug(1, "NO don't press that button...");
  redis_ipc_send_debug(0, "I told you not to press it!!");
  redis_ipc_cleanup(gettid());

  return 0;
}

int main(int argc, char **argv)
{
  pthread_t printer_thread_info, button_thread_info;

  pthread_create(&printer_thread_info, NULL, run_printer_thread, NULL);
  pthread_create(&button_thread_info, NULL, run_button_thread, NULL);

  pthread_join(printer_thread_info, NULL);
  pthread_join(button_thread_info, NULL);

  return 0;
}
