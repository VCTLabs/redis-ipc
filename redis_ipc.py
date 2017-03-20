# This is a Python module to provide client access to the Redis server
# it is treated as a library

import os
import redis
import json
import time
import pdb   ########## debug  ( no kidding)
# global data
"""
"""

# global functions
def pdic2jdic(pdic):
    """
    pdic   -    a Python dictionary
    
    returns a JSON dictionary
    """
    if type(pdic) != type({}):
        raise NotDict
    try:
        jd=json.dumps(pdic)
    except:
        raise BadMessage
    return jd
    
def jdic2pdic(jdic):
    """
    jdic     -    a JSON string which is a hash
    
    returns a Python dictionary
    """
    try:
        pd=json.loads(jdic)
    except,(TypeError,ValueError):
        raise BadMessage
    if type(pd) != type({}):
        raise NotDict
    return pd

def redis_connect():
    """
    attempt to open a connection to the Redis server
    raise an exception if this does not work
    return the connection object if it does work
    """
    conn=redis.Connection()
    try:
        conn.connect()
    except redis.ConnectionError:
         del conn
         raise NoRedis
    del conn
    sr=redis.StrictRedis()
    return sr
    
# exceptions
class RedisIpcExc(Exception):
    pass
NoRedis     =  RedisIpcExc("redis server not available")
NotDict     =  RedisIpcExc("redis message was not a Python dictionary")
BadMessage  =  RedisIpcExc("redis message not a recognizable message")
MsgTimeout  =  RedisIpcExc("redis message request timed out")

# the main feature here is a class which will provide the wanted access
class redis_client(object):
    
    def __init__(self,component,thread="main"):
        """
        component : friendly name for calling program
                    (e.g. how it is labeled on system architecture diagrams
                     as opposed to exact executable name)
        thread: friendly name for specific thread of execution,
                allowing IPC from multiple threads in a multi-threaded program
        """

        self.component=component
        self.thread=thread

        # process number of this component (a python program)
        self.process_number=os.getpid()

        # construct name of queue where replies to commands should arrive
        self.results_queue = "queues.results.%s.%s" % (component, thread)

        # initialize redis connection
        self.redis_conn=redis_ipc.redis_connect()

    def __generate_msg_id(self):
        # unique id for message
        # component name, process number, timestamp
        ts=str(time.time())   # floating timestamp
        the_id=self.component + ':' + self.process_number + ':' + ts
        return the_id,ts
        
    def redis_ipc_send_and_receive(self, dest, cmd, tmout):
        """
        dest     -   name of the component to handle this command (string)
        cmd      -   the command to send (a Python dictionary)
        tmout    -   timeout for receiving a response, floating seconds
        """
        # add standard fields to the command dictionary
        late_news=self.__generate_msg_id()  # id and timestamp
        cmd["timestamp"] = late_news[1]  # just the timestamp
        cmd["component"] = self.component
        cmd["thread"] = self.thread
        cmd["tid"] = self.process_number
        cmd["results_queue"] = self.results_queue
        cmd["command_id"] = late_news[0]  # the id includes the timestamp

        # calculate name of command queue
        dest_queue = "queues.commands.%s" % dest

        # send off the command message  # still a Python dictionary
        self.__redis_ipc_send_command(dest_queue, cmd)

        # wait on results queue for the answer
        # an exception is raised by the request function if it times out
        response=self.__redis_ipc_receive_reply(cmd, tmout)
        return response
        
    def __redis_ipc_send_command(self, dest_queue, cmd):
        """
        arguments are mandatory
        dest_queue -   command queue serviced by destination component 
        cmd        -   a command known to the receiving component
                     
        this routine does not block
        it just sends the command to the back of the queue
        """
        # turn command into a JSON dictionary before sending it
        msg=pdic2jdic(cmd)

        # send it via Redis
        self.redis_conn(dest_queue,msg)  # no waiting

    def __redis_ipc_receive_reply(self, cmd, tmout):
        """
        arguments are mandatory
        cmd           - command for which we await a reply
        tmout         - timeout for receiving a response, floating seconds
        
        a proper response is a JSON dictionary
        turn it into a Python dictionary
        
        if the request timed out, the response is empty, 
        and an exception will be raised
        if a non-empty value was received, if it is not the response
        to the specified command
           try again
        else 
           return this result
        
        """
        
        # use self.results_queue as name of queue to wait on
        # throw out received messages until reply["command_id"] == cmd["command_id"]
        got_it=False
        while not got_it:
            jrep=None
            jrep=self.redis_conn.blpop()               
            if jrep==None:
                raise MsgTimeout
            prep=redis_ipc.jdic2pdic(jrep)
            if prep["command_id"] != cmd["command_id"]:
                continue  # skip this message, not our response
            # take it
            return prep  # good enough
            
            
class redis_server(object):
    
    def __init__(self, component):
        """
        component : friendly name for calling program
                    (e.g. how it is labeled on system architecture diagrams
                     as opposed to exact executable name)
        """

        self.component=component

        # process number of this component (a python program)
        self.process_number=os.getpid()

        # construct name of queue where commands should arrive
        self.command_queue = "queues.commands.%s." % component

        # initialize redis connection
        self.redis_conn=redis_ipc.redis_connect()

    def redis_ipc_receive_command(self):
        """
        blocks for command to arrive in own command queue, 
        return it as Python dictionary
        """
        # get serialized command message
        jcmd=self.redis_conn.blpop(self.command_queue)
        pcmd=redic_ipc.jdic2pdic(jcmd)
        return pcmd
        
    def redis_ipc_send_reply(self, cmd, result):
        """
        arguments are mandatory
        cmd    - command that was processed so result is now available
        result - the generated result
                     
        this routine does not block
        it just sends the reply to the back of the queue
        """

        # command contains name of reply queue
        dest_queue = cmd["results_queue"]

        # tie reply to its command with matching command_id
        result["command_id"] = cmd["command_id"]

        # turn command into a JSON dictionary before sending it
        msg=pdic2jdic(result)

        # send it via Redis
        self.redis_conn.rpush(dest_queue, msg)