import zmq
import time
import sys
#import pycrfseg as pymmseg
import pymmseg
from zmq_common import *

'''
 s, len(24bit), text
 response:
 r, len(24bit), offset should be divide.
 e, len(24bit), eror message.
'''

def main(zq_addr, dict_path):
    context = zmq.Context()
    socket = context.socket(zmq.REP)
    #socket.bind("ipc:///tmp/example2")
    socket.bind(zq_addr)

    # init tokenizer.
    m = pymmseg.mmseg()
    m.init(dict_path)
    error_resp = make_error("Invalid Command")
    while True:
        #  Wait for next request from client
        req = socket.recv()
        # print "Received request: ", message
        resp = error_resp
        try:
            sText = parse_token_request(req)
            rs = m.split(sText)
            print sText
            for i in rs:
                print i, 
            resp = make_token_reponse(rs)
        except InvalidCommandError, ex:
            pass # default route.
        socket.send(resp)


if __name__ == '__main__':
    zq_addr = sys.argv[1]
    dict_path = sys.argv[2]
    main(zq_addr, dict_path)

