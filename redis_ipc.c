#define _GNU_SOURCE // for gettid()
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/syscall.h>
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

// gettid() is missing a libc wrapper for some reason
// (manpage even mentions it)
static pid_t gettid()
{
  return syscall(SYS_gettid);
}

struct redis_ipc_per_thread * get_per_thread_info()
{
    struct redis_ipc_per_thread *next_info = redis_ipc_info;
//@@@@@@ FIXME: switch to using a list, entry for each thread 
    //while (iterate through thread info list)
    if (next_info->tid == gettid())
    {
        return next_info; 
    }

    return NULL;
}

// NOTE: keep the following type enum and names array in sync

enum redis_ipc_type
{
    RPC_TYPE_INVALID = 0,
    RPC_TYPE_SETTING,
    RPC_TYPE_STATUS,
    RPC_TYPE_COMMAND,
    RPC_TYPE_RESULT,
    RPC_TYPE_EVENT,
    RPC_TYPE_DEBUG
};

const char *redis_ipc_type_names[] = 
{
   "INVALID",
   "settings",
   "status",
   "commands",
   "results",
   "events",
   "debug"
};

// generate redis key names aka our IPC "paths" 
static int ipc_path(char *buf, size_t buf_len, enum redis_ipc_type type, 
                    const char *component, 
                    // only used for "results" type
                    const char *thread, 
                    // only used for "events" type
                    const char *subchannel)
{
    int ret = RIPC_FAIL, path_len = -1;

    switch (type)
    {
        case RPC_TYPE_SETTING:
        case RPC_TYPE_STATUS:
            path_len = snprintf(buf, buf_len, "%s.%s", 
                                redis_ipc_type_names[type],
                                component);
            break;

        case RPC_TYPE_COMMAND:
            path_len = snprintf(buf, buf_len, "queues.%s.%s", 
                                component,
                                redis_ipc_type_names[type]);
            break;

        case RPC_TYPE_RESULT:
            if (thread != NULL)
            {
                path_len = snprintf(buf, buf_len, "queues.%s.%s.%s", 
                                    component, thread, 
                                    redis_ipc_type_names[type]);
            }
            break;

        case RPC_TYPE_EVENT:
        case RPC_TYPE_DEBUG:
            if (subchannel != NULL)
            {
                path_len = snprintf(buf, buf_len, "channel.%s.%s.%s", 
                                    component, subchannel, 
                                    redis_ipc_type_names[type]);
            }
            else
            {
                path_len = snprintf(buf, buf_len, "channel.%s.%s", 
                                    component, redis_ipc_type_names[type]);
            }
            break;
    }

    // path_len == buf_len indicates truncation
    if (path_len > 0 && path_len < buf_len)
    {
        ret = RIPC_OK;
    }

    return ret;
}

void cleanup_per_thread_info(struct redis_ipc_per_thread *thread_info)
{
    safe_free(thread_info->component);
    safe_free(thread_info->thread);
    safe_free(thread_info->result_queue_path);
    if (thread_info->redis_state)
    {
        // closes connection and frees memory related to this connection
        redisFree(thread_info->redis_state);
    }
}

// initialize per-thread state 
int redis_ipc_init(const char *this_component, const char *this_thread)
{
    struct redis_ipc_per_thread *new_info = NULL;
    char result_queue_path[RIPC_MAX_IPC_PATH_LEN];
    int ret = RIPC_FAIL;

    new_info = calloc(1, sizeof(struct redis_ipc_per_thread));

    // component is name for all threads/processes in same subsystem,
    // thread is uniquely (non-random) assigned name for this thread,
    new_info->component = strdup(this_component);
    new_info->thread = strdup(this_thread);
    if (new_info->component == NULL || new_info->thread == NULL)
        goto redis_ipc_init_failed;

    // tid is OS thread ID number
    new_info->tid = gettid();

    // each thread gets its own redis connection
    new_info->redis_state = redisConnect(RIPC_SERVER_IP, RIPC_SERVER_PORT);
    if (new_info->redis_state == NULL || new_info->redis_state->err)
        goto redis_ipc_init_failed;

    // each thread gets its own results queue in redis
    memset(result_queue_path, 0, sizeof(result_queue_path));
    ret = ipc_path(result_queue_path, sizeof(result_queue_path),
                  RPC_TYPE_RESULT, this_component, this_thread, NULL);
    if (ret != RIPC_OK)
        goto redis_ipc_init_failed;
    new_info->result_queue_path = strdup(result_queue_path);
    if (new_info->result_queue_path == NULL)
        goto redis_ipc_init_failed;

    redis_ipc_info = new_info; //@@@@@@ FIXME: switch to using a list, append entry for each new thread

    return RIPC_OK;

redis_ipc_init_failed:
    cleanup_per_thread_info(new_info);
    safe_free(new_info);

    return RIPC_FAIL;
}


int format_debug_msg(char *msg, size_t max_msg_len, 
                     const char *format, va_list argp)
{
    const char *trunc_warning = "[TRUNC]";
    size_t warning_len = strlen(trunc_warning);
    size_t warning_offset = max_msg_len - warning_len - 1;
    int msg_len = -1;
    int ret = RIPC_FAIL;

    // give up if message buffer is ridiculously small
    if (max_msg_len < warning_len+1) return RIPC_FAIL;

    msg_len = vsnprintf(msg, max_msg_len, format, argp);

    // flag truncation of message
    if (msg_len == max_msg_len)
    {
        // write warning over end of message
        strncpy(&msg[warning_offset], trunc_warning, warning_len);
    }

    if (msg_len > 0)
    {
        ret = RIPC_OK;
    }

    return ret;
}

//@@@@ FIXME: debug will be dynamically configurable from a setting or config file
int get_debug_verbosity()
{
    return 5;
}

//@@@@ FIXME: debug will be dynamically configurable from a setting or config file
int stderr_debug_is_enabled()
{
    return 1;
}

// run redis command and reset connection upon error
//
// if return value is non-null,
// caller is responsible for calling freeReplyObject() when done with it
redisReply * redis_command(const char *format, ...)
{
    struct redis_ipc_per_thread *thread_info = get_per_thread_info();
    redisReply *reply = NULL;
    va_list argp;
    
    if (stderr_debug_is_enabled())
    {
        va_start(argp, format);
        vfprintf(stderr, format, argp);
        fprintf(stderr, "\n");
        va_end(argp);
    }

    va_start(argp, format);
    reply = redisvCommand(thread_info->redis_state, format, argp);
    va_end(argp);

    if (reply == NULL)
    {
        // must reconnect to redis server after an error 
        redisFree(thread_info->redis_state);
        thread_info->redis_state = redisConnect(RIPC_SERVER_IP, RIPC_SERVER_PORT);
    }

    return reply;
}

static int redis_publish(const char *channel_path, json_object *obj)
{
    const char *json_text = json_object_to_json_string(obj);
    int ret = RIPC_FAIL;
    redisReply *reply = NULL;

    // don't forget to free reply
    reply = redis_command("PUBLISH %s %s", channel_path, json_text);

    if (reply != NULL)
    {
        ret = RIPC_OK;
        freeReplyObject(reply);
    }

    return ret;
}

int redis_ipc_send_debug(unsigned int debug_level, const char *format, ...)
{
    char msg_buffer[RIPC_MAX_DEBUG_LEN];
    char debug_channel_path[RIPC_MAX_IPC_PATH_LEN];
    va_list argp;
    struct redis_ipc_per_thread *thread_info = get_per_thread_info();
    json_object *debug_obj = NULL;
    int ret = RIPC_FAIL;

    // ignore if the debug level is higher than current verbosity
    if (debug_level > get_debug_verbosity())
    {
        return RIPC_OK;  // it's easy to successfully do nothing...
    }

    // format message into string
    memset(msg_buffer, 0, sizeof(msg_buffer));
    va_start(argp, format);
    ret = format_debug_msg(msg_buffer, sizeof(msg_buffer), format, argp);
    va_end(argp);
    if (ret != RIPC_OK)
        goto redis_ipc_send_debug_finish;

    // generate debug object
    debug_obj = json_object_new_object();
    if (debug_obj == NULL)
        goto redis_ipc_send_debug_finish;
    json_object_object_add(debug_obj, "message", 
                           json_object_new_string(msg_buffer));
    json_object_object_add(debug_obj, "component", 
                           json_object_new_string(thread_info->component));
    json_object_object_add(debug_obj, "thread", 
                           json_object_new_string(thread_info->thread));
    json_object_object_add(debug_obj, "level", 
                           json_object_new_int(debug_level));
//@@@ FIXME: add timestamp
//    json_object_object_add(debug_obj, "timestamp", 
//                    json_object_new_string(timestamp_buffer);

    // publish debug object
    ret = ipc_path(debug_channel_path, sizeof(debug_channel_path),
                   RPC_TYPE_DEBUG, thread_info->component, NULL, NULL);
    if (ret != RIPC_OK)
        goto redis_ipc_send_debug_finish;
    ret = redis_publish(debug_channel_path, debug_obj);

redis_ipc_send_debug_finish:
   json_object_put(debug_obj);  // safe to call even if NULL

   return ret;
}
