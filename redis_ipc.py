# This is a Python module to provide client access to the Redis server
# it is treated as a library

import redis
import time
import pdb   ########## debug  ( no kidding)
# global data
"""
"""

# global functions
def is_json_msg(m):   # do we need this???
    global component
    pass
 
# exceptions
class RedisIpcExc(Exception):
    pass
NoRedis     =  RedisIpcExc("redis server not available")
BadMessage  =  RedisIpcExc("redis message not a recognizable message")
MsgTimeout  =  RedisIpcExc("redis message request timed out")

# the main feature here is a class which will provide the wanted access
class redis_client(object):
    
    def __init__(self,component):
        # component : queue name for listening
        self.component=component
        # @@@@@@@@@@@@(process number of this component)
        self.process_number=str(1234)

    def generate_msg_id(self):
        # unique id for message
        # component name, process number, timestamp
        ts=str(time.time())   # floating timestamp
        proc=99   # @@@@@@@@@@@@@@@@
        the_id=self.component + ':' + self.process_number + ':' + ts
        return the_id
        
    def redis_ipc_send_and_receive(self,dest,cmd,tmout):
        """
        dest     -   the name of a component queue (string)
        cmd      -   the command to send (a Python dictionary)
        tmout    -   timeout for receiving a response, floating seconds
                     this method calls the method 
                     redis_ipc_receive_command_blocking
                     with this timeout
                     
        calls the (unblocking) redis_ipc_send_command
        for the actual send
        """
        self.redis_ipc_send(dest, cmd)
        
    def redis_ipc_send(self,dest, cmd):
        """
        arguments are mandatory
        dest     -   queue name for recipient queue
        cmd      -   a command known to the receiving component
                     it is a JSON dictionary
                     
        this routine does not block
        it just sends the command to the back of the queue
        """
        # the command is a Python dictionary
        # turn it into a JSON dictionary before sending it
        # send it via Redis
        pass

    def redis_ipc_receive_command_blocking(self,qq,tmout):
        """
        arguments are mandatory
        qq the components receive queue
        tmout   -   timeout for receiving a response, floating seconds
        
        the response is a JSON dictionary
        turn it into a Python dictionary
        return it
        """
        pass
        
   
        
    