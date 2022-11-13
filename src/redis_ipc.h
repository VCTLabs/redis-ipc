// Copyright (c) 2011-2021 Vanguard Computer Technology Labs <answers@vctlabs.com>
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef __REDIS_IPC_H__
#define __REDIS_IPC_H__

#include <sys/types.h>
#include <json-c/json.h>

#ifdef __cplusplus
extern "C" {
#endif


//*************
// constants
//*************

// debug levels for redis_ipc_send_debug()
enum { RIPC_DBG_ERROR,
       RIPC_DBG_ALERT,
       RIPC_DBG_WARN,
       RIPC_DBG_INFO,
       RIPC_DBG_NOISY,
       RIPC_DBG_EXTRA_NOISY };

// return codes > 0 are also OK
#define RIPC_OK     0
#define RIPC_FAIL (-1)

// location of config file
// #define RIPC_CONF_PATH "/etc/redis_ipc.conf"
#define RIPC_CONF_PATH "./redis_ipc.conf"

//*********** things that might move to config file when it is implemented

// max length of formatted debug message (will be truncated if longer)
#define RIPC_MAX_DEBUG_LEN 1024

// max length of constructed redis key name
#define RIPC_MAX_IPC_PATH_LEN 128

// redis server address
#ifdef RIPC_RUNTIME_DIR
#pragma message "RIPC_RUNTIME_DIR was set"
#else
#pragma message "RIPC_RUNTIME_DIR was *not* set"
#define RIPC_RUNTIME_DIR "/tmp/redis-ipc"
#endif
#define RIPC_SERVER_PATH RIPC_RUNTIME_DIR "/socket"

//*********** defaults for settings that can be optionally configured via functions

// default verbosity level for debug channel messages
#define RIPC_DEFAULT_VERBOSITY 5

// default stderr debug messages enabled
#define RIPC_DEFAULT_STDERR 1

// component allowed to write settings (or "*" if any component is allowed)
#define RIPC_COMPONENT_ANY "*"
#define RIPC_DEFAULT_SETTINGS_WRITER "db"
// #define RIPC_DEFAULT_SETTINGS_WRITER RIPC_COMPONENT_ANY

//*************
// functions
//*************

// When a function returns an int, return value RIPC_OK indicates success;
// when a function returns a json_object*, non-null return indicates success.

// NOTE: whenever a json_object* is used as a return value or a parameter,
//       the caller is responsible for cleaning it up with json_object_put().
//       This cleanup function is safe to call on a NULL pointer.
//       When a char* is used as a return value, caller is responsible for
//       cleaning it up with free().

// Init should be called from each thread that will be doing redis IPC,
// with all threads getting same component name but unique thread name.
// Cleanup is provided to prevent a slow leak from turnover of short-lived
// threads that each call init to allocate a few per-thread variables.
//
// The thread name should indicate the purpose of the thread and be
// predictable, e.g. live-http-worker-3, rather than randomly generated or
// based on TID number, because it will be used to generate name of results
// queue -- and we want same results queues to be re-used if component gets
// restarted (avoid having to garbage-collect stale, abandoned redis queues)
//
// Cleanup is done based on thread ID so that main thread can clean up for
// terminated threads, as long as it is tracking their IDs. For a
// single-threaded process, tid == pid.

int redis_ipc_init(const char *this_component, const char *this_thread);
int redis_ipc_cleanup(pid_t tid);


// A component can send a command to any other component,
// but it can only receive commands from its own command queue(s).
//
// A component sending a command should supply NULL as subqueue unless
// it has multiple command queues defined in redis_ipc.conf,
// in which case it should supply one of those queues. The same goes
// for a component waiting for commands -- if it only has one command
// queue, subqueue should be NULL.
//
// The timeout field is in seconds, or use zero to indicate no timeout
// for blocking.
//
// When submitting a command, the name of submitter's results queue
// and a unique command ID will automatically be inserted into the command
// as fields "results_queue" and "command_id" respectively.
//
// The submitter will block on its results until result has been received
// or timeout has been exceeded. If a result with _different_ command ID
// is received, it will be logged as an error to submitter component's
// debug channel then freed, and the wait on results queue will restart
// (expiration time will be reset to original).

int get_debug_verbosity();
int stderr_debug_is_enabled();

// After receiving and executing a command, the receiving component
// should submit a result. The original command is passed in as a parameter
// to provide the command ID and path to results queue. The command ID
// will automatically be added to the result object before pushing it to the
// result queue so that the command submitter will only get back the
// expected result (as opposed to a stale result, belonging to a command that
// took so long that submitter timed out before seeing the result).

json_object * redis_ipc_send_command_blocking(const char *dest_component,
                                              const char *subqueue,
                                              json_object *command,
                                              unsigned int timeout);
json_object * redis_ipc_receive_command_blocking(const char *subqueue,
                                              unsigned int timeout);
int redis_ipc_send_result(const json_object *completed_command, json_object *result);


// A component can only write a setting if it has been authorized,
// but it can read any setting.
//
// Each setting is a set of fields that are key-value pairs, where both field name and
// values are stored as strings (of course values could be JSON text if needed).
//
// A setting can be written or read in its entirety using the first pair of functions:
// when writing, the JSON object parameter can hold all the component's setting fields
// as key-value pairs (although it would also work with a smaller set of fields);
// when reading, the JSON object returned *will* hold all existing setting fields.
//
// A setting can also be written or read a single field at a time using the second
// pair of functions, which works on strings rather JSON hashes. If multiple setting
// fields are being accessed, consider accessing the full thing rather than multiple
// calls to individual fields -- it may be more efficient.

int redis_ipc_write_setting(const char *owner_component, const json_object *fields);
json_object * redis_ipc_read_setting(const char *owner_component);
int redis_ipc_write_setting_field(const char *owner_component, const char *field_name,
                                  const char *field_value);
char * redis_ipc_read_setting_field(const char *owner_component, const char *field_name);


// A component can only write its own status,
// but it can read any status.
//
// Each status is a set of fields that are key-value pairs, where both field name and
// values are stored as strings (of course values could be JSON text if needed).
//
// A status can be written or read in its entirety using the first pair of functions:
// when writing, the JSON object parameter can hold all the component's status fields
// as key-value pairs (although it would also work with a smaller set of fields);
// when reading, the JSON object returned *will* hold all existing status fields.
//
// A status can also be written or read a single field at a time using the second
// pair of functions, which works on strings rather JSON hashes. If multiple status
// fields are being accessed, consider accessing the full thing rather than multiple
// calls to individual fields -- it may be more efficient.

int redis_ipc_write_status(const json_object *fields);
json_object * redis_ipc_read_status(const char *owner_component);
int redis_ipc_write_status_field(const char *field_name, const char *field_value);
char * redis_ipc_read_status_field(const char *owner_component, const char *field_name);


// Each component can only send event messages to its own event channel(s),
// but can subscribe to any (or all) event channels.
//
// A component sending an event should supply NULL as subchannel unless
// it has multiple subchannels defined in redis_ipc.conf,
// in which case it should supply one of those subchannels.
//
// A component subscribing to events can use NULL as component parameter
// to watch all event channels for all components. Or, to watch all events
// from a single component, use NULL for subchannel (as mentioned above,
// not all components even have subchannels).
//
// The send function will automatically append the following fields,
// plus the standard ones (timestamp, etc) to the event:
//   channel (full channel name)
//
// The unsubscribe function stops watching all event channels.

int redis_ipc_send_event(const char *subchannel, json_object *message);
int redis_ipc_subscribe_events(const char *component, const char *subchannel);
int redis_ipc_unsubscribe_events(void);


// Each component will send debug messages to its own debug channel,
// but can subscribe to any (or all) debug channels. Use NULL as
// component to subscribe to all debug channels.
//
// Debug send can serve as drop-in replacement for printf-style logging,
// which is why it doesn't take a JSON object. The send function internally
// generates a json object containing following fields, plus the standard
// ones (timestamp, etc)
//   message
//   level
//   channel (full channel name)
//
// When sending debug messages, low debug level indicates
// high message priority, since only messages of lower or equal level
// to configured component debug verbosity will actually get sent.
//
// The unsubscribe function stops watching all debug channels.

int redis_ipc_send_debug(unsigned int debug_level, const char *format, ...);
int redis_ipc_subscribe_debug(const char *component);
int redis_ipc_unsubscribe_debug(void);


// This function is the counterpart to both redis_ipc_send_event()
// and redis_ipc_send_debug() because a received message can come from any
// subscribed channel.
//
// A thread listening for published messages will block indefinitely until the
// next event is received, because redis protocol does not implement a timeout
// on waiting for messages. To prevent infinite blocking, at least one channel
// should have been subscribed before listening for messages.

json_object * redis_ipc_get_message_blocking(void);

// Functions for configuring behavior of redis-ipc itself
// see RIPC_DEFAULT_* definitions for default values if not called

// configure verbosity level for debug channel
int redis_ipc_config_debug_verbosity(int verbosity);

// configure whether debug messages will be shown on stderr
int redis_ipc_config_stderr_debug(int enable_stderr);

// configure which component will be authorized to write settings
// (RIPC_COMPONENT_ANY works as wildcard for "any")
int redis_ipc_config_settings_writer(const char *writer_component);

#ifdef __cplusplus
}
#endif

#endif  // __REDIS_IPC_H__
