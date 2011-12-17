
#ifndef __REDIS_IPC_H__
#define __REDIS_IPC_H__

#include <sys/types.h>
#include <json/json.h>

//*************
// constants
//*************

// return codes > 0 are also OK
#define RIPC_OK     0
#define RIPC_FAIL (-1)

// location of config file
//#define RIPC_CONF_PATH "/etc/redis_ipc.conf"
#define RIPC_CONF_PATH "./redis_ipc.conf"

//*********** things that might move to config file when it is implemented

// max length of formatted debug message (will be truncated if longer)
#define RIPC_MAX_DEBUG_LEN 1024

// max length of constructed redis key name
#define RIPC_MAX_IPC_PATH_LEN 128

// redis server address
#define RIPC_SERVER_IP "127.0.0.1"
#define RIPC_SERVER_PORT 6379

//*************
// functions
//*************

// Init should be called from each thread that will be doing redis IPC,
// with all threads getting same component name but unique thread name.
// Cleanup is provided to prevent a slow leak from turnover of short-lived
// threads that each call init to allocate a few per-thread variables.
// Cleanup is done based on thread ID so that main thread can clean up for
// terminated threads.
//
// The thread name should indicate the purpose of the thread and be 
// predictable, e.g. live-http-worker-3, rather than randomly generated or
// based on TID number, because it will be used to generate name of results
// queue -- and we want same results queues to be re-used if component gets
// restarted (avoid having to garbage-collect stale, abandoned redis queues)
int redis_ipc_init(const char *this_component, const char *this_thread);
int redis_ipc_cleanup(pid_t tid);

// A component can send a command to any other component, 
// but it can only receive commands from its own command queue(s).
//
// The timeout field is in seconds, or use zero to indicate no timeout
// for blocking.
//
// When submitting a command, the name of submitter's results queue 
// and the command ID will automatically be inserted into the command. 
//
// The submitter will block on its results until reply has been received 
// or timeout has been exceeded. If a reply with _different_ command ID
// is received, it will be logged as an error to submitter component's 
// debug channel then freed, and the wait on results queue will restart
// (expiration time will be reset to original).
//
// After receiving and executing a command, the receiving component
// should submit a reply. The original command is passed in as a parameter
// to provide the command ID and path to results queue. The command ID
// will automatically be added to the reply object before pushing it to the
// result queue so that the submitter can verify that this reply is the
// expected one (as opposed to a stale reply, belonging to a command that 
// took so long that submitter timed out before seeing the reply).
json_object * redis_ipc_send_command_blocking(const char *dest_component, 
                                              const char *queue_name, 
                                              unsigned int command_id, 
                                              json_object *command, 
                                              unsigned int timeout);
json_object * redis_ipc_receive_command_blocking(const char *queue_name,
                                              unsigned int timeout);
int redis_ipc_send_reply(const json_object *completed_command, json_object *reply);
                        

// A component can only write a setting if it has been authorized by redis_ipc.conf, 
// but it can read any setting.
//
// If multiple status fields are being accessed, probably should access the full thing
// rather than multiple calls to individual fields.
int redis_ipc_write_setting(const char *owner_component, json_object *value);
json_object * redis_ipc_read_setting(const char *owner_component);
int redis_ipc_write_setting_field(const char *owner_component, const char *field_name, json_object *value);
json_object * redis_ipc_read_setting_field(const char *owner_component, const char *field_name);

// A component can only write its own status, 
// but it can read any status.
//
// If multiple status fields are being accessed, probably should access the full thing
// rather than multiple calls to individual fields.
int redis_ipc_write_status(json_object *value);
json_object * redis_ipc_read_status(const char *owner_component);
int redis_ipc_write_status_field(const char *field_name, json_object *value);
json_object * redis_ipc_read_status_field(const char *owner_component, const char *field_name);

// Each component will send debug messages to its own debug channel,
// but can subscribe to any (or all) debug channels.
//
// A component sending an event should supply NULL as subchannel unless
// it has multiple subchannels defined in redis_ipc.conf,
// in which case it should supply one of those subchannels. The name of
// the channel to which an event is posted will automatically get added 
// to the message as "source" field so that subscribers to multiple 
// channels can easily determine which channel a message came from.
//
// A component subscribing to events can use NULL as component parameter
// to watch all event channels for all components. Or, to watch all events
// from a single component, use NULL for subchannel (as mentioned above,
// not all components even have subchannels).
// 
// A thread watching for events will block indefinitely until the next
// event is received, because redis protocol does not implement a timeout
// on waiting for messages. 
//
// The unsubscribe function stops watching all event channels.
int redis_ipc_send_event(const char *subchannel, json_object *message);
int redis_ipc_subscribe_events(const char *component, const char *subchannel);
json_object * redis_ipc_watch_events_blocking();
int redis_ipc_unsubscribe_events();

// Each component will send debug messages to its own debug channel,
// but can subscribe to any (or all) debug channels. Use NULL as
// component to subscribe to all debug channels.
//
// Debug send can serve as drop-in replacement for printf-style logging,
// which is why it doesn't take a JSON object. The send function internally
// generates a json object containing a "message" field and a "timestamp" 
// field. The component and thread posting the message will automatically be 
// added to the object so that subscribers to multiple debug channels can 
// easily determine who sent the message.

// When sending debug messages, low debug level indicates
// high message priority, since only messages of lower or equal level
// to configured component debug verbosity will actually get sent.
// 
// A thread watching for debug messages will block indefinitely until the next
// debug message is received, because redis protocol does not implement 
// a timeout on waiting for messages. 
//
// The unsubscribe function stops watching all debug channels.
int redis_ipc_send_debug(unsigned int debug_level, const char *format, ...);
int redis_ipc_subscribe_debug(const char *component);
json_object * redis_ipc_receive_debug_blocking();
int redis_ipc_unsubscribe_debug();
#endif
