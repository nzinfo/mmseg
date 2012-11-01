import sys
import zmq
from zmq_common import *

def sender(zq_addr):
    context = zmq.Context()

    #  Socket to talk to server
    socket = context.socket(zmq.REQ)
    #socket.connect ("ipc:///tmp/example2")
    socket.connect(zq_addr)

    #  Do 10 requests, waiting each time for a response
    msg = "hello world! test ok."
    req =  make_token_request(msg)
    socket.send (req)

    #  Get the reply.
    message = socket.recv()
    rs = parse_token_reponse(message)
    #print "Received reply ", req , "[", message, "]"
    for i in rs:
       print msg[i:]

if __name__ == '__main__':
    zq_addr = sys.argv[1]
    sender( zq_addr )

