print("abc".."efg")

local ffi = require("ffi")
ffi.cdef[[
int barfunc(int foo);
]]
local barreturn = ffi.C.barfunc(253)
io.write(barreturn)
io.write('\n')

function f (x, y)
   return (x^2 * math.sin(y))/(1 - x)
end
