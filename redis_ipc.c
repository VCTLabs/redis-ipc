#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <json/json.h>
#include <hiredis/hiredis.h>
#include "redis_ipc.h"

#define safe_free(ptr) { if (ptr) free(ptr); ptr = NULL; }

struct redis_ipc_per_thread
{
//@@@@ FIXME: this will turn into a linked list
//  list_t list_head
    pid_t tid;               // owning thread
    char *component;         // component name used in redis
    char *thread;            // thread name used in redis
    char *result_queue_path; // based on component and thread
    redisContext *redis_state; // state for connection to redis server
};

static struct redis_ipc_per_thread *redis_ipc_info = NULL;

void cleanup_thread(struct redis_ipc_per_thread *thread_info)
{
    safe_free(thread_info->component);
    safe_free(thread_info->thread);
    safe_free(thread_info->result_queue_path);
    if (thread_info->redis_state)
    {
        redisFree(thread_info->redis_state);
    }
}

// initialize per-thread state 
int redis_ipc_init(const char *this_component, const char *this_thread)
{
    struct redis_ipc_per_thread *new_info = NULL;
    char result_queue_path[RIPC_MAX_REDIS_NAME];

    new_info = calloc(1, sizeof(struct redis_ipc_per_thread));

    // component is name for all threads/processes in same subsystem,
    // thread is uniquely (non-random) assigned name for this thread,
    new_info->component = strdup(this_component);
    new_info->thread = strdup(this_thread);
    if (new_info->component == NULL || new_info->thread == NULL)
    {
        goto redis_ipc_init_failed;
    }

    // tid is OS thread ID number
    new_info->tid = gettid();

    // each thread gets its own redis connection
    new_info->redis_state = redisConnect(RIPC_SERVER_IP, RIPC_SERVER_PORT);
    if (new_info->redis_state == NULL || new_info->redis_state->err)
    {
        goto redis_ipc_init_failed;
    }

    // each thread gets its own results queue in redis
    memset(result_queue_path, 0, sizeof(result_queue_path));
    snprintf(result_queue_path, sizeof(result_queue_path), 
             "queues.%s.%s.results", this_component, this_thread);
    new_info->result_queue_path = strdup(result_queue_path);
    if (new_info->result_queue_path == NULL)
    {
        goto redis_ipc_init_failed;
    }

    redis_ipc_info = new_info; //@@@@@@ FIXME: switch to using a list, append entry for each new thread

    return RIPC_OK;

redis_ipc_init_failed:
    cleanup_thread(new_info);
    safe_free(new_info);

    return RIPC_FAIL;
}

// returns length of formatted message
int format_debug_msg(char *msg, size_t max_msg_len, 
                     const char *format, va_list argp)
{
    const char *trunc_warning = "[TRUNC]";
    size_t warning_len = strlen(trunc_warning);
    size_t warning_offset = max_msg_len - warning_len - 1;
    int msg_len = -1;

    // give up if message buffer is ridiculously small
    if (max_msg_len < warning_len+1) return 0;

    msg_len = vsnprintf(msg, max_msg_len, format, argp);

    // flag truncation of message
    if (msg_len == max_msg_len)
    {
        // write warning over end of message
        strncpy(&msg[warning_offset], trunc_warning, warning_len);
    }

    return msg_len;
}

//@@@@ FIXME: debug will be dynamically configurable from a setting 
int current_debug_verbosity()
{
    return 5;
}

int redis_ipc_send_debug(unsigned int debug_level, const char *format, ...)
{
    char msg_buffer[RIPC_MAX_DEBUG_LENGTH];
    va_list argp;

    // ignore if the debug level is higher than current verbosity
    if (debug_level > current_debug_verbosity())
    {
        return RIPC_OK;  // it's easy to successfully do nothing...
    }

    // format message into string
    memset(msg_buffer, 0, sizeof(msg_buffer));
    va_start(argp, format);
    format_debug_msg(msg_buffer, sizeof(msg_buffer), format, argp);
    va_end(argp);

    // generate debug object

    // publish debug object
}
