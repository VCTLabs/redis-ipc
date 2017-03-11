# This is a Python module to provide client access to the Redis server
# it is treated as a library

# global data
"""
components
queues
"""
# global functions
def is_json_msg(m):   # do we need this???
    pass

# the main feature here is a class which will provide the wanted access
class redis_client(object):
    def __init__(self,compo):
        self.component=compo
        self.cfg=self.config(cfg_file=None)
        
    def redis_ipc_send_command_blocking(self,dest,cmd,tmout):
        pass
        
    def redis_ipc_receive_command_blocking(self,qq,tmout):
        pass
        
    def redis_ipc_send_result(self,cmd_reply,cmd_result):
        pass
        

        
    