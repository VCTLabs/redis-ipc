# This is a Python module to provide client access to the Redis server
# it is treated as a library

# globals  ???

# the main feature here is a class which will provide the wanted access
class redis_client(object):
    def __init__(compo):
        self.component=compo
        
    def redis_ipc_send_command_blocking(dest,cmd,tmout):
        pass
        
    def redis_ipc_receive_command_blocking(qq,tmout):
        pass
        
    def redis_ipc_send_result(cmd_reply,cmd_result):
        pass
        
    def is_json_msg():   # do we need this???
        pass