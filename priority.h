//*************
// functions
//*************

// A component can send a command to any other component, 
// but it can only receive commands from its own command queue(s).
//

@@@ NOTE we do not need to worry about subqueus, one command queue
    per component will do just fine

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
//
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
