#define _GNU_SOURCE // for gettid()
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <json-c/json.h>
#include <hiredis/hiredis.h>
#include "redis_ipc.h"

#define safe_free(ptr) { if (ptr) free(ptr); ptr = NULL; }

struct redis_ipc_per_thread
{
    pid_t tid;               // owning thread
    char *component;         // component name used in redis
    char *thread;            // thread name used in redis
    char *result_queue_path; // based on component and thread
    redisContext *redis_state; // state for connection to redis server
    unsigned int command_ctr;  // counter for number of commands sent by thread
};

static __thread struct redis_ipc_per_thread *redis_ipc_info = NULL;

// gettid() is missing a libc wrapper before glibc 2.30
// (manpage even mentions it)
#if (__GLIBC_MINOR__ < 30)
pid_t gettid()
{
  return syscall(SYS_gettid);
}
#endif

struct redis_ipc_per_thread * get_per_thread_info()
{
    struct redis_ipc_per_thread *next_info = redis_ipc_info;

    if (next_info != NULL && next_info->tid == gettid())
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
   "queues.commands",
   "queues.results",
   "channel.events",
   "channel.debug"
};

// generate redis key names aka our IPC "paths" 
//
// extra_path arg: 
//   not used for "settings", "status" types,
//   mandatory thread name for "results" type, 
//   optional subqueue name for "commands" type,
//   optional subchannel name for "events" type
//
// all callers are internal and expected to enforce above conventions
static int ipc_path(char *buf, size_t buf_len, enum redis_ipc_type type, 
                    const char *component, 
                    const char *extra_path) 
{
    int ret = RIPC_FAIL, path_len = -1;
    const char *extra_path_separator = NULL;

    if (extra_path != NULL) 
    {
        extra_path_separator = ".";
    }
    else
    {
        extra_path = "";
        extra_path_separator = "";
    }

    switch (type)
    {
        case RPC_TYPE_SETTING:
        case RPC_TYPE_STATUS:
        case RPC_TYPE_COMMAND:
        case RPC_TYPE_RESULT:
        case RPC_TYPE_EVENT:
        case RPC_TYPE_DEBUG:
            // last 2 args will be empty strings unless needed to distinguish
            // among multiple queues/channels of component
            path_len = snprintf(buf, buf_len, "%s.%s%s%s", 
                                redis_ipc_type_names[type],
                                component, extra_path_separator,
                                extra_path);
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

    if (new_info == NULL)
        goto redis_ipc_init_failed;

    // component is name for all threads/processes in same subsystem,
    // thread is uniquely (non-random) assigned name for this thread,
    new_info->component = strdup(this_component);
    new_info->thread = strdup(this_thread);
    if (new_info->component == NULL || new_info->thread == NULL)
        goto redis_ipc_init_failed;

    // tid is OS thread ID number
    new_info->tid = gettid();

    // each thread gets its own redis connection
    new_info->redis_state = redisConnectUnix(RIPC_SERVER_PATH);
    if (new_info->redis_state == NULL || new_info->redis_state->err)
        goto redis_ipc_init_failed;

    // each thread gets its own results queue in redis
    memset(result_queue_path, 0, sizeof(result_queue_path));
    ret = ipc_path(result_queue_path, sizeof(result_queue_path),
                  RPC_TYPE_RESULT, this_component, this_thread);
    if (ret != RIPC_OK)
        goto redis_ipc_init_failed;
    new_info->result_queue_path = strdup(result_queue_path);
    if (new_info->result_queue_path == NULL)
        goto redis_ipc_init_failed;

    redis_ipc_info = new_info; 

    return RIPC_OK;

redis_ipc_init_failed:
    if (stderr_debug_is_enabled())
    {
            fprintf(stderr, "[ERROR] redis_ipc_init failed for thread %s\n", this_thread);
    }
    cleanup_per_thread_info(new_info);
    safe_free(new_info);

    return RIPC_FAIL;
}

int redis_ipc_cleanup(pid_t tid)
{
    struct redis_ipc_per_thread *next_info = redis_ipc_info;
    int ret = RIPC_FAIL;

    if (next_info != NULL && next_info->tid == tid)
    {
        cleanup_per_thread_info(next_info);
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

// check for errors in redis command execution;
// reset connection or free reply object if error is found
redisReply * validate_redis_reply(redisReply *reply)
{
    struct redis_ipc_per_thread *thread_info = get_per_thread_info();
    redisReply *validated_reply = reply;

    // error in connection to server
    if (reply == NULL)
    {
        // must reconnect to redis server after an error 
        if (stderr_debug_is_enabled())
        {
            fprintf(stderr, "[ERROR] '%s', need to reconnect\n",
                    thread_info->redis_state->errstr);
        }
        redisFree(thread_info->redis_state);
        thread_info->redis_state = redisConnectUnix(RIPC_SERVER_PATH);
    }
    // error in command
    else if (reply->type == REDIS_REPLY_ERROR)
    {
        if (stderr_debug_is_enabled())
        {
            fprintf(stderr, "[ERROR] command failed: %s\n", reply->str);
        }
        freeReplyObject(validated_reply);
        validated_reply = NULL;
    }

    return validated_reply;
}

// run redis command and return reply object if command succeeds
//
// if return value is non-null,
// caller is responsible for calling freeReplyObject() when done with reply
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

    // check for redis errors and avoid returning an error string
    // (returns NULL instead, harder to overlook...)
    reply = validate_redis_reply(reply);

    return reply;
}

// run redis command and return reply object if command succeeds
//
// if return value is non-null,
// caller is responsible for calling freeReplyObject() when done with reply
redisReply * redis_command_from_list(int argc, const char **argv)
{
    struct redis_ipc_per_thread *thread_info = get_per_thread_info();
    redisReply *reply = NULL;
    va_list argp;
    int i = 0;
    
    if (stderr_debug_is_enabled())
    {
        for (i = 0; i < argc; i++)
        {
            fprintf(stderr, "%s ", argv[i]);
        }
        fprintf(stderr, "\n");
    }

    reply = redisCommandArgv(thread_info->redis_state, argc, argv, NULL);

    // check for redis errors and avoid returning an error string
    // (returns NULL instead, harder to overlook...)
    reply = validate_redis_reply(reply);

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


static int redis_push(const char *queue_path, json_object *obj)
{
    const char *json_text = NULL;
    redisReply *reply = NULL;
    int ret = RIPC_FAIL;

    // append standard fields (such as timestamp)
    json_add_common_fields(obj);

    json_text = json_object_to_json_string(obj);

    // don't forget to free reply later
    reply = redis_command("RPUSH %s %s", queue_path, json_text);

    if (reply != NULL)
    {
        ret = RIPC_OK;
        freeReplyObject(reply);
    }

    return ret;
}

static json_object * redis_pop(const char *queue_path, unsigned int timeout)
{
    const char *json_text = NULL;
    json_object *entry = NULL;
    redisReply *reply = NULL;
    int ret = RIPC_FAIL;

    // don't forget to free reply later
    reply = redis_command("BLPOP %s %d", queue_path, timeout);
    if (reply == NULL)
        goto redis_pop_finish;

    // extract queue entry from reply object
    //
    // reply should be an array: 
    //   string <queue path>
    //   string <entry> 
    if (reply->type != REDIS_REPLY_ARRAY)
        goto redis_pop_finish;
    if (reply->element[1]->type != REDIS_REPLY_STRING)
        goto redis_pop_finish;

    json_text = reply->element[1]->str;

    if (stderr_debug_is_enabled())
    {
        fprintf(stderr, "[ENTRY:%s] %s\n", queue_path, json_text);
    }

    // parse popped entry back into json object
    entry = json_tokener_parse(json_text);

redis_pop_finish:
    if (reply != NULL)
        freeReplyObject(reply);

    return entry;
}

json_object * redis_ipc_send_command_blocking(const char *dest_component, 
                                              const char *subqueue, 
                                              json_object *command, 
                                              unsigned int timeout)
{
    char command_queue_path[RIPC_MAX_IPC_PATH_LEN];
    char result_queue_path[RIPC_MAX_IPC_PATH_LEN];
    char id_buffer[RIPC_MAX_IPC_PATH_LEN];
    struct redis_ipc_per_thread *thread_info = get_per_thread_info();
    json_object *result = NULL, *result_id_obj = NULL;
    const char *result_id_str = NULL;
    int ret = RIPC_FAIL, received_result = 0;

    // make sure successful init has been performed
    if (thread_info == NULL)
        goto redis_ipc_send_command_blocking_finish;

    // calculate name of command queue belonging to specified component
    ret = ipc_path(command_queue_path, sizeof(command_queue_path),
                   RPC_TYPE_COMMAND, dest_component, subqueue);
    if (ret != RIPC_OK)
        goto redis_ipc_send_command_blocking_finish;

    // calculate name of own queue for receiving the result
    ret = ipc_path(result_queue_path, sizeof(result_queue_path),
                   RPC_TYPE_RESULT, thread_info->component, thread_info->thread);
    if (ret != RIPC_OK)
        goto redis_ipc_send_command_blocking_finish;

    // generate unique id used to match command and its result 
    snprintf(id_buffer, sizeof(id_buffer), "%s-%s-%d-%u", 
             thread_info->component, thread_info->thread, 
             thread_info->tid, thread_info->command_ctr++);

    // append generated fields to command
    json_object_object_add(command, "results_queue", 
                           json_object_new_string(result_queue_path));
    json_object_object_add(command, "command_id", 
                           json_object_new_string(id_buffer));

    // push command onto queue
    ret = redis_push(command_queue_path, command);
    if (ret != RIPC_OK)
        goto redis_ipc_send_command_blocking_finish;

    // receive result
    while (!received_result)
    {
        // wait for entry from results queue
        result = redis_pop(result_queue_path, timeout);
        if ( result != NULL && !json_object_is_type(result, json_type_object))
        {
            // results are supposed to be hash objects, not whatever this thing is
            redis_ipc_send_debug(RIPC_DBG_ERROR, "redis_ipc_send_command_blocking():"
                                 "invalid result '%s'", json_object_get_string(result));
            json_object_put(result);
            result = NULL;
        }
        if (result == NULL)
            goto redis_ipc_send_command_blocking_finish;

        // extract command id from received result
        result_id_obj = json_object_object_get(result, "command_id");
        if (result_id_obj == NULL)
            goto redis_ipc_send_command_blocking_finish;
        result_id_str = json_object_get_string(result_id_obj);
        if (result_id_str == NULL)
            goto redis_ipc_send_command_blocking_finish;

        // verify result matches submitted command
        if (strcmp(result_id_str, id_buffer) == 0)
        {
            received_result = 1; 
        }
        else
        {
           redis_ipc_send_debug(RIPC_DBG_ERROR, "redis_ipc_send_command_blocking():"
                                   "received stale result id %s, want id %s",
                                result_id_str, id_buffer);
        }
    }

redis_ipc_send_command_blocking_finish:
    if (result == NULL || ! json_object_is_type(result, json_type_object))
    {
        redis_ipc_send_debug(RIPC_DBG_ERROR, "redis_ipc_send_command_blocking():"
                                "invalid result");
    }

    return result;
}

json_object * redis_ipc_receive_command_blocking(const char *subqueue,
                                                 unsigned int timeout)
{
    char command_queue_path[RIPC_MAX_IPC_PATH_LEN];
    struct redis_ipc_per_thread *thread_info = get_per_thread_info();
    json_object *command = NULL;
    int ret = RIPC_FAIL;

    // make sure successful init has been performed
    if (thread_info == NULL)
        goto redis_ipc_receive_command_blocking_finish;

    // calculate name of own command queue 
    ret = ipc_path(command_queue_path, sizeof(command_queue_path),
                   RPC_TYPE_COMMAND, thread_info->component, subqueue);
    if (ret != RIPC_OK)
        goto redis_ipc_receive_command_blocking_finish;

    // wait for entry from command queue
    command = redis_pop(command_queue_path, timeout);
    if ( command != NULL && !json_object_is_type(command, json_type_object))
    {
        // commands are supposed to be hash objects, not whatever this thing is
        redis_ipc_send_debug(RIPC_DBG_ERROR, "redis_ipc_receive_command_blocking():"
                             "invalid command '%s'", json_object_get_string(command));
        json_object_put(command);
        command = NULL;
    }

redis_ipc_receive_command_blocking_finish:

    return command;
}

int redis_ipc_send_result(const json_object *completed_command, json_object *result)
{
    json_object *id_obj = NULL, *result_queue_obj = NULL;
    const char *id_str = NULL, *result_queue_path = NULL;
    struct redis_ipc_per_thread *thread_info = get_per_thread_info();
    int ret = RIPC_FAIL;

    // make sure successful init has been performed
    if (thread_info == NULL)
        goto redis_ipc_send_result_finish;

    // extract name of result queue 
    result_queue_obj = json_object_object_get(completed_command, "results_queue");
    if (result_queue_obj == NULL)
        goto redis_ipc_send_result_finish;
    result_queue_path = json_object_get_string(result_queue_obj);

    // extract command id
    id_obj = json_object_object_get(completed_command, "command_id");
    if (id_obj == NULL)
        goto redis_ipc_send_result_finish;
    id_str = json_object_get_string(id_obj);

    // append command id to result
    json_object_object_add(result, "command_id", 
                           json_object_new_string(id_str));

    // push result onto queue
    ret = redis_push(result_queue_path, result);

redis_ipc_send_result_finish:
    if (ret != RIPC_OK)
        redis_ipc_send_debug(RIPC_DBG_ERROR, "redis_ipc_send_result():"
                                "could not send result for command %s",
                                json_object_get_string(completed_command));

    return ret;
}

int get_field_count(json_object *obj)
{
    int num_fields = 0;

    json_object_object_foreach(obj, key, val)
    {
        num_fields++;
    }

    return num_fields;
}

int redis_write_hash(const char *hash_path, json_object *obj)
{
    int argc = 0, num_fields = 0;
    const char **argv = NULL;
    redisReply *reply = NULL;
    int ret = RIPC_FAIL;

    // find out how many fields we need to pass into redis
    num_fields = get_field_count(obj);

    // fill out redis command to set multiple fields
    argv = calloc(2 + num_fields*2, sizeof(char *)); // cmd, hash, keys & values
    argv[argc] = "HMSET";
    argc++;
    argv[argc] = hash_path;
    argc++;
    json_object_object_foreach(obj, key, val)
    {
        argv[argc] = key;
        argc++;
        argv[argc] = json_object_get_string(val);
        argc++;
    }

    // don't forget to free reply later
    reply = redis_command_from_list(argc, argv);

    if (reply != NULL)
    {
        ret = RIPC_OK;
        freeReplyObject(reply);
    }

    return ret;
}
                        
//@@@@ FIXME: name of component(s) allowed to change settings will be in config file
static int component_can_write_settings(const char *component)
{
    return (strcmp(component, "db") == 0);
}

int redis_ipc_write_setting(const char *owner_component, const json_object *fields)
{
    char setting_hash_path[RIPC_MAX_IPC_PATH_LEN];
    struct redis_ipc_per_thread *thread_info = get_per_thread_info();
    int ret = RIPC_FAIL;

    // make sure successful init has been performed
    if (thread_info == NULL)
        goto redis_ipc_write_setting_finish;

    // verify this component is allowed to update settings
    if (!component_can_write_settings(thread_info->component))
        goto redis_ipc_write_setting_finish;

    // calculate name of setting hash corresponding to specified component
    ret = ipc_path(setting_hash_path, sizeof(setting_hash_path),
                   RPC_TYPE_SETTING, owner_component, NULL);
    if (ret != RIPC_OK)
        goto redis_ipc_write_setting_finish;

    // update current setting hash with supplied values
    ret = redis_write_hash(setting_hash_path, fields);

redis_ipc_write_setting_finish:

    return ret;
}

int redis_ipc_write_status(const json_object *fields)
{
    char status_hash_path[RIPC_MAX_IPC_PATH_LEN];
    struct redis_ipc_per_thread *thread_info = get_per_thread_info();
    int ret = RIPC_FAIL;

    // make sure successful init has been performed
    if (thread_info == NULL)
        goto redis_ipc_write_status_finish;

    // calculate name of own status hash
    ret = ipc_path(status_hash_path, sizeof(status_hash_path),
                   RPC_TYPE_STATUS, thread_info->component, NULL);
    if (ret != RIPC_OK)
        goto redis_ipc_write_status_finish;

    // update current status hash with supplied values
    ret = redis_write_hash(status_hash_path, fields);

redis_ipc_write_status_finish:

    return ret;
}

int redis_write_hash_field(const char *hash_path, const char *field_name, 
                           const char *field_value)
{
    redisReply *reply = NULL;
    int ret = RIPC_FAIL;

    // don't forget to free reply later
    reply = redis_command("HSET %s %s %s", hash_path, field_name, field_value);

    if (reply != NULL)
    {
        ret = RIPC_OK;
        freeReplyObject(reply);
    }

    return ret;
}

int redis_ipc_write_setting_field(const char *owner_component, const char *field_name, 
                                  const char *field_value)
{
    char setting_hash_path[RIPC_MAX_IPC_PATH_LEN];
    struct redis_ipc_per_thread *thread_info = get_per_thread_info();
    int ret = RIPC_FAIL;

    // make sure successful init has been performed
    if (thread_info == NULL)
        goto redis_ipc_write_setting_field_finish;

    // verify this component is allowed to update settings
    if (!component_can_write_settings(thread_info->component))
        goto redis_ipc_write_setting_field_finish;

    // calculate name of setting hash corresponding to specified component
    ret = ipc_path(setting_hash_path, sizeof(setting_hash_path),
                   RPC_TYPE_SETTING, owner_component, NULL);
    if (ret != RIPC_OK)
        goto redis_ipc_write_setting_field_finish;

    // update current setting hash with supplied field 
    ret = redis_write_hash_field(setting_hash_path, field_name, field_value);

redis_ipc_write_setting_field_finish:

    return ret;
}

int redis_ipc_write_status_field(const char *field_name, const char *field_value)
{
    char status_hash_path[RIPC_MAX_IPC_PATH_LEN];
    struct redis_ipc_per_thread *thread_info = get_per_thread_info();
    int ret = RIPC_FAIL;

    // make sure successful init has been performed
    if (thread_info == NULL)
        goto redis_ipc_write_status_field_finish;

    // calculate name of own status hash
    ret = ipc_path(status_hash_path, sizeof(status_hash_path),
                   RPC_TYPE_STATUS, thread_info->component, NULL);
    if (ret != RIPC_OK)
        goto redis_ipc_write_status_field_finish;

    // update current status hash with supplied field 
    ret = redis_write_hash_field(status_hash_path, field_name, field_value);

redis_ipc_write_status_field_finish:

    return ret;
}

json_object * redis_read_hash(const char *hash_path)
{
    redisReply *reply = NULL;
    json_object *fields = NULL;
    const char *key = NULL, *val = NULL;
    int i = 0;

    // don't forget to free reply later
    reply = redis_command("HGETALL %s", hash_path);

    // extract fields from reply object
    //
    // reply should be an array with alternating field names and values

    fields = json_object_new_object();
    if (fields == NULL)
        goto redis_read_hash_finish;
    if (reply == NULL)
        goto redis_read_hash_finish;
    if (reply->type != REDIS_REPLY_ARRAY)
        goto redis_read_hash_finish;
    if (reply->elements % 2) // need to have name-value pairs 
        goto redis_read_hash_finish;

    if (stderr_debug_is_enabled()) fprintf(stderr, "[HASH]");
    while (i < reply->elements)
    {
        key = reply->element[i++]->str;
        val = reply->element[i++]->str;
        json_object_object_add(fields, key, json_object_new_string(val));

        if (stderr_debug_is_enabled()) fprintf(stderr, " %s='%s'", key, val);
    }
    if (stderr_debug_is_enabled()) fprintf(stderr, "\n");

redis_read_hash_finish:
    if (reply != NULL)
        freeReplyObject(reply);

    return fields;
}

json_object * redis_ipc_read_setting(const char *owner_component)
{
    char setting_hash_path[RIPC_MAX_IPC_PATH_LEN];
    json_object *fields = NULL;
    struct redis_ipc_per_thread *thread_info = get_per_thread_info();
    int ret = RIPC_FAIL;

    // make sure successful init has been performed
    if (thread_info == NULL)
        goto redis_ipc_read_setting_finish;

    // calculate name of setting hash belonging to specified component
    ret = ipc_path(setting_hash_path, sizeof(setting_hash_path),
                   RPC_TYPE_SETTING, owner_component, NULL);
    if (ret != RIPC_OK)
        goto redis_ipc_read_setting_finish;

    // get all fields of setting hash 
    fields = redis_read_hash(setting_hash_path);

redis_ipc_read_setting_finish:

    return fields;
}

json_object * redis_ipc_read_status(const char *owner_component)
{
    char status_hash_path[RIPC_MAX_IPC_PATH_LEN];
    json_object *fields = NULL;
    struct redis_ipc_per_thread *thread_info = get_per_thread_info();
    int ret = RIPC_FAIL;

    // make sure successful init has been performed
    if (thread_info == NULL)
        goto redis_ipc_read_status_finish;

    // calculate name of status hash belonging to specified component
    ret = ipc_path(status_hash_path, sizeof(status_hash_path),
                   RPC_TYPE_STATUS, owner_component, NULL);
    if (ret != RIPC_OK)
        goto redis_ipc_read_status_finish;

    // get all fields of status hash 
    fields = redis_read_hash(status_hash_path);

redis_ipc_read_status_finish:

    return fields;
}

char * redis_read_hash_field(const char *hash_path, const char *field_name) 
{
    redisReply *reply = NULL;
    char *field_value = NULL;
    int ret = RIPC_FAIL;

    // don't forget to free reply later
    reply = redis_command("HGET %s %s", hash_path, field_name);

    // extract fields from reply object
    //
    // reply should be a string
    if (reply == NULL)
    {
        if (stderr_debug_is_enabled()) fprintf(stderr, "[HASH_FIELD] <null result>\n");
        goto redis_read_hash_field_finish;
    }
    if (reply->type != REDIS_REPLY_STRING)
    {
        if (stderr_debug_is_enabled()) fprintf(stderr, "[HASH_FIELD] <non-string result type %d>\n", reply->type);
        goto redis_read_hash_field_finish;
    }
    field_value = strdup(reply->str);
    if (stderr_debug_is_enabled()) fprintf(stderr, "[HASH_FIELD] %s='%s'\n", field_name, field_value);

redis_read_hash_field_finish:
    if (reply != NULL)
        freeReplyObject(reply);

    return field_value;
}

char * redis_ipc_read_setting_field(const char *owner_component, const char *field_name)
{
    char setting_hash_path[RIPC_MAX_IPC_PATH_LEN];
    char *field_value = NULL;
    struct redis_ipc_per_thread *thread_info = get_per_thread_info();
    int ret = RIPC_FAIL;

    // make sure successful init has been performed
    if (thread_info == NULL)
        goto redis_ipc_read_setting_field_finish;

    // calculate name of setting hash belonging to specified component
    ret = ipc_path(setting_hash_path, sizeof(setting_hash_path),
                   RPC_TYPE_SETTING, owner_component, NULL);
    if (ret != RIPC_OK)
        goto redis_ipc_read_setting_field_finish;

    // get all fields of setting hash 
    field_value = redis_read_hash_field(setting_hash_path, field_name);

redis_ipc_read_setting_field_finish:

    return field_value;
}

char * redis_ipc_read_status_field(const char *owner_component, const char *field_name)
{
    char status_hash_path[RIPC_MAX_IPC_PATH_LEN];
    const char *field_value = NULL;
    struct redis_ipc_per_thread *thread_info = get_per_thread_info();
    int ret = RIPC_FAIL;

    // make sure successful init has been performed
    if (thread_info == NULL)
        goto redis_ipc_read_status_field_finish;

    // calculate name of status hash belonging to specified component
    ret = ipc_path(status_hash_path, sizeof(status_hash_path),
                   RPC_TYPE_STATUS, owner_component, NULL);
    if (ret != RIPC_OK)
        goto redis_ipc_read_status_field_finish;

    // get all fields of status hash 
    field_value = redis_read_hash_field(status_hash_path, field_name);

redis_ipc_read_status_field_finish:

    return field_value;
}

static int redis_publish(const char *channel_path, json_object *obj)
{
    const char *json_text = NULL;
    redisReply *reply = NULL;
    int ret = RIPC_FAIL;

    // append channel path
    json_object_object_add(obj, "channel", 
                           json_object_new_string(channel_path));

    // append standard fields (such as timestamp)
    json_add_common_fields(obj);

    json_text = json_object_to_json_string(obj);

    // don't forget to free reply later
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

    // make sure successful init has been performed
    if (thread_info == NULL)
        goto redis_ipc_send_event_finish;

    // calculate channel name
    ret = ipc_path(event_channel_path, sizeof(event_channel_path),
                   RPC_TYPE_EVENT, thread_info->component, subchannel);
    if (ret != RIPC_OK)
        goto redis_ipc_send_event_finish;

    // publish event object
    ret = redis_publish(event_channel_path, message);

redis_ipc_send_event_finish:

   return ret;
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


int redis_ipc_send_debug(unsigned int debug_level, const char *format, ...)
{
    char msg_buffer[RIPC_MAX_DEBUG_LEN];
    char debug_channel_path[RIPC_MAX_IPC_PATH_LEN];
    va_list argp;
    struct redis_ipc_per_thread *thread_info = get_per_thread_info();
    json_object *debug_obj = NULL;
    int ret = RIPC_FAIL;

    // make sure successful init has been performed
    if (thread_info == NULL)
        goto redis_ipc_send_debug_finish;

    // ignore if the debug level is higher than current verbosity
    if (debug_level > get_debug_verbosity())
    {
        return RIPC_OK;  // it's easy to successfully do nothing...
    }

    // calculate channel name
    ret = ipc_path(debug_channel_path, sizeof(debug_channel_path),
                   RPC_TYPE_DEBUG, thread_info->component, NULL);
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
    redisReply *reply = NULL;
    int ret = RIPC_FAIL;

    // don't forget to free reply later
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
    struct redis_ipc_per_thread *thread_info = get_per_thread_info();
    int ret = RIPC_FAIL;

    // make sure successful init has been performed
    if (thread_info == NULL)
        goto redis_ipc_subscribe_events_finish;

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
                   RPC_TYPE_EVENT, component_pattern, subchannel);
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
    struct redis_ipc_per_thread *thread_info = get_per_thread_info();
    int ret = RIPC_FAIL;

    // make sure successful init has been performed
    if (thread_info == NULL)
        goto redis_ipc_subscribe_debug_finish;

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
                   RPC_TYPE_DEBUG, component_pattern, NULL);
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

    // don't forget to free reply later
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
    struct redis_ipc_per_thread *thread_info = get_per_thread_info();
    int ret = RIPC_FAIL;

    // make sure successful init has been performed
    if (thread_info == NULL)
        goto redis_ipc_unsubscribe_events_finish;

    ret = ipc_path(event_channel_path, sizeof(event_channel_path),
                   RPC_TYPE_EVENT, "*", NULL);
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
    struct redis_ipc_per_thread *thread_info = get_per_thread_info();
    int ret = RIPC_FAIL;

    // make sure successful init has been performed
    if (thread_info == NULL)
        goto redis_ipc_unsubscribe_debug_finish;

    ret = ipc_path(debug_channel_path, sizeof(debug_channel_path),
                   RPC_TYPE_DEBUG, "*", NULL);
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

    // make sure successful init has been performed
    if (thread_info == NULL)
        goto redis_get_channel_message_finish;

    // block until a message is available
    ret = redisGetReply(thread_info->redis_state, &reply);
    if (ret != REDIS_OK)
    {
        // must reconnect to redis server after an error 
        redisFree(thread_info->redis_state);
        thread_info->redis_state = redisConnectUnix(RIPC_SERVER_PATH);

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
        fprintf(stderr, "[MESSAGE] %s\n", message_str);
    }

    // parse message back into json object
    message = json_tokener_parse(message_str);

redis_get_channel_message_finish:
    if (reply != NULL)
        freeReplyObject(reply);

   return message;
}
