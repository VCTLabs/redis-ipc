#define _GNU_SOURCE // for gettid()
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
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

// standard fields for json objects that will be useful when
// troubleshooting component integration issues, if nothing else...
#define MAX_TIMESTAMP_LEN 64 
void json_add_common_fields(json_object *obj)
{
    struct redis_ipc_per_thread *thread_info = get_per_thread_info();
    char timestamp_buffer[MAX_TIMESTAMP_LEN];
    struct timeval timestamp;

    // collect timestamp as string
    memset(&timestamp, 0, sizeof(timestamp));
    memset(timestamp_buffer, 0, sizeof(timestamp_buffer));
    gettimeofday(&timestamp, NULL);
    snprintf(timestamp_buffer, sizeof(timestamp_buffer), "%ld.%06ld", 
             timestamp.tv_sec, timestamp.tv_usec);

    json_object_object_add(obj, "timestamp", 
                           json_object_new_string(timestamp_buffer));
    json_object_object_add(obj, "component", 
                           json_object_new_string(thread_info->component));
    json_object_object_add(obj, "thread", 
                           json_object_new_string(thread_info->thread));
    json_object_object_add(obj, "tid", json_object_new_int(gettid()));
}

static int redis_publish(const char *channel_path, json_object *obj)
{
    const char *json_text = NULL;
    int ret = RIPC_FAIL;
    redisReply *reply = NULL;

    // append channel path
    json_object_object_add(obj, "channel", 
                           json_object_new_string(channel_path));

    // append standard fields (such as timestamp)
    json_add_common_fields(obj);

    json_text = json_object_to_json_string(obj);

    // don't forget to free reply
    reply = redis_command("PUBLISH %s %s", channel_path, json_text);

    if (reply != NULL)
    {
        ret = RIPC_OK;
        freeReplyObject(reply);
    }

    return ret;
}

// caller is responsible for cleaning up the message object
int redis_ipc_send_event(const char *subchannel, json_object *message)
{
    char event_channel_path[RIPC_MAX_IPC_PATH_LEN];
    struct redis_ipc_per_thread *thread_info = get_per_thread_info();
    int ret = RIPC_FAIL;

    // calculate channel name
    ret = ipc_path(event_channel_path, sizeof(event_channel_path),
                   RPC_TYPE_EVENT, thread_info->component, NULL, subchannel);
    if (ret != RIPC_OK)
        goto redis_ipc_send_event_finish;

    // publish event object
    ret = redis_publish(event_channel_path, message);

redis_ipc_send_event_finish:

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

    // calculate channel name
    ret = ipc_path(debug_channel_path, sizeof(debug_channel_path),
                   RPC_TYPE_DEBUG, thread_info->component, NULL, NULL);
    if (ret != RIPC_OK)
        goto redis_ipc_send_debug_finish;

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
    json_object_object_add(debug_obj, "level", 
                           json_object_new_int(debug_level));

    // publish debug object
    ret = redis_publish(debug_channel_path, debug_obj);

redis_ipc_send_debug_finish:
   json_object_put(debug_obj);  // safe to call even if NULL

   return ret;
}

static int redis_subscribe(const char *channel_path)
{
    int ret = RIPC_FAIL;
    redisReply *reply = NULL;

    // don't forget to free reply
    reply = redis_command("PSUBSCRIBE %s", channel_path);

    if (reply != NULL)
    {
        ret = RIPC_OK;
        freeReplyObject(reply);
    }

    return ret;
}

// if component is NULL, subscribe to all event channels
// if only subchannel is NULL, subscribe to all event channels for component
// (for some components there may only be one event channel)
int redis_ipc_subscribe_events(const char *component, const char *subchannel)
{
    char event_channel_path[RIPC_MAX_IPC_PATH_LEN];
    char *component_pattern = NULL;
    size_t pattern_len = 0;
    int ret = RIPC_FAIL;

    // calculate channel name
    if (component == NULL) 
    {
        // use wildcard to get all event channels
        component_pattern = strdup("*");
    }
    else
    {
        if (subchannel == NULL)
        {
            // append wildcard to get all event channels for this component
            pattern_len = strlen(component)+2;
            component_pattern = calloc(1, pattern_len);
            snprintf(component_pattern, pattern_len, "%s%c", component, '*');
        }
        else
        {
            // exact match for both component and subchannel
            component_pattern = strdup(component);
        }
    }
    ret = ipc_path(event_channel_path, sizeof(event_channel_path),
                   RPC_TYPE_EVENT, component_pattern, NULL, subchannel);
    if (ret != RIPC_OK)
        goto redis_ipc_subscribe_events_finish;

    // publish event object
    ret = redis_subscribe(event_channel_path);

redis_ipc_subscribe_events_finish:
   safe_free(component_pattern);

   return ret;
}

int redis_ipc_subscribe_debug(const char *component)
{
    char debug_channel_path[RIPC_MAX_IPC_PATH_LEN];
    const char *component_pattern = NULL;
    size_t pattern_len = 0;
    int ret = RIPC_FAIL;

    // calculate channel name
    if (component == NULL) 
    {
        // use wildcard to get all debug channels
        component_pattern = "*";
    }
    else
    {
        // exact match 
        component_pattern = component;
    }
    ret = ipc_path(debug_channel_path, sizeof(debug_channel_path),
                   RPC_TYPE_DEBUG, component_pattern, NULL, NULL);
    if (ret != RIPC_OK)
        goto redis_ipc_subscribe_debug_finish;

    // publish debug object
    ret = redis_subscribe(debug_channel_path);

redis_ipc_subscribe_debug_finish:

   return ret;
}

static int redis_unsubscribe(char *channel_path)
{
    redisReply *reply = NULL;
    int ret = RIPC_FAIL;

    // don't forget to free reply
    reply = redis_command("PUNSUBSCRIBE %s", channel_path);

    if (reply != NULL)
    {
        ret = RIPC_OK;
        freeReplyObject(reply);
    }

    return ret;
}

// unsubscribe from all event channels
int redis_ipc_unsubscribe_events()
{
    char event_channel_path[RIPC_MAX_IPC_PATH_LEN];
    int ret = RIPC_FAIL;

    ret = ipc_path(event_channel_path, sizeof(event_channel_path),
                   RPC_TYPE_EVENT, "*", NULL, NULL);
    if (ret != RIPC_OK)
        goto redis_ipc_unsubscribe_events_finish;

    ret = redis_unsubscribe(event_channel_path);

redis_ipc_unsubscribe_events_finish:

   return ret;
}

// unsubscribe from all debug channels
int redis_ipc_unsubscribe_debug()
{
    char debug_channel_path[RIPC_MAX_IPC_PATH_LEN];
    int ret = RIPC_FAIL;

    ret = ipc_path(debug_channel_path, sizeof(debug_channel_path),
                   RPC_TYPE_DEBUG, "*", NULL, NULL);
    if (ret != RIPC_OK)
        goto redis_ipc_unsubscribe_debug_finish;

    ret = redis_unsubscribe(debug_channel_path);

redis_ipc_unsubscribe_debug_finish:

   return ret;
}

// current thread should be subscribed to one or more callers before calling
// caller is responsible for cleaning up the returned message object
json_object * redis_ipc_get_message_blocking(void)
{
    json_object *message = NULL;
    redisReply *reply = NULL;
    const char *message_str = NULL;
    struct redis_ipc_per_thread *thread_info = get_per_thread_info();
    int ret = RIPC_FAIL;

    // block until a message is available
    ret = redisGetReply(thread_info->redis_state, &reply);
    if (ret != REDIS_OK)
    {
        // must reconnect to redis server after an error 
        redisFree(thread_info->redis_state);
        thread_info->redis_state = redisConnect(RIPC_SERVER_IP, RIPC_SERVER_PORT);

        goto redis_get_channel_message_finish;
    }


    // extract message from reply object
    //
    // reply should be an array: 
    //   string "pmessage" 
    //   string <psubscribe pattern>
    //   string <sending channel>
    //   string <message> ** entry of interest
    if (reply->type != REDIS_REPLY_ARRAY)
        goto redis_get_channel_message_finish;
    if (reply->element[3]->type != REDIS_REPLY_STRING)
        goto redis_get_channel_message_finish;

    message_str = reply->element[3]->str;

    if (stderr_debug_is_enabled())
    {
        fprintf(stderr, "MESSAGE %s\n", message_str);
    }

    // parse message back into json object
    message = json_tokener_parse(message_str);
    if (is_error(message)) message = NULL;

redis_get_channel_message_finish:
    if (reply != NULL)
        freeReplyObject(reply);

   return message;
}
