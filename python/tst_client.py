import zmq

context = zmq.Context()

#  Socket to talk to server
print "Connecting to hello world server..."
socket = context.socket(zmq.REQ)
socket.connect ("ipc:///tmp/example2")

#  Do 10 requests, waiting each time for a response
for request in range (1,10):
    print "Sending request ", request,"..."
    socket.send ("Hello")

    #  Get the reply.
    message = socket.recv()
    print "Received reply ", request, "[", message, "]"
