print("abc".."efg")

local ffi = require("ffi")
ffi.cdef[[
	
	int strncmp ( const char * str1, const char * str2, size_t num );

    typedef struct _token_ctx {
        int a;
        char val[255];
    }token_ctx;
    //typedef int (*charlevel_callback_proto)(const char*, int);
    typedef int (__stdcall *charlevel_callback_proto)(token_ctx* ctx, const char*, int n);
    int must_call_callback(charlevel_callback_proto func, const char* msg);
	int barfunc(int foo);
]]
local barreturn = ffi.C.barfunc(253)
io.write(barreturn)
io.write('\n')

function f (x, y)
   return (x^2 * math.sin(y))/(1 - x)
end

local C = ffi.C


local cb = ffi.cast("charlevel_callback_proto", function(ctx, msg, n)
  -- ffi.string will alloc new memory; if wanna for speed, try use c API's string compare method.
  print(ffi.string ( msg ) )
  print("world")
  print(C.strncmp("hello world.", msg, 10))
  return 0
end )

C.must_call_callback(cb, "hello world.")

cb:free()