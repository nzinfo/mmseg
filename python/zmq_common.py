import struct
import ctypes

class InvalidCommandError(Exception): pass

def make_token_request(aText):
    if type(aText) == unicode:
       aText = aText.encode('utf-8')
    aText = str(aText)
    op_type = 'S'
    op_len = len(aText)
    header = (op_len << 8) + ord(op_type)
    return struct.pack('I', header) + aText

def parse_token_request(req):
    #print req
    '''
       = check is valid command, only 'S' is supported.
         - if not raise an exception.
         
    '''
    v = req[:4]
    v = struct.unpack('I', v)
    #print v
    v = v[0]
    cmd = v & 0xFF
    if chr(cmd) != 'S':
       raise  InvalidCommandError()
    vlen = v >> 8
    return req[4:]


def make_token_reponse(aSegResult):
    rs = aSegResult
    arr = (ctypes.c_int * len(rs)) (*rs)
    print len(arr)

    op_type = 'R'
    op_len = len(arr)
    header = (op_len << 8) + ord(op_type)
    return struct.pack('I'+'I'*len(rs), header, *rs )

def parse_token_reponse(resp):
    v = resp[:4]
    v = struct.unpack('I', v)
    #print v
    v = v[0]
    cmd = v & 0xFF
    if chr(cmd) != 'R':
       raise  InvalidCommandError()
    vlen = v >> 8
    return struct.unpack('I'*vlen, resp[4:])
    #return resp[4:]

def make_error(aErrMsgText):
    pass

