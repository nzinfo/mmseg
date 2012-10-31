import struct
op_type = 'B'
op_len  = 4399
v = op_len << 8;
v |= v + ord(op_type)
print v

#decode
cv = v & 0xFF
print chr(cv) , v >> 8
#print ord('B'), chr(66)
#v = struct.pack('B
