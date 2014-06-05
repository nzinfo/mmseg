local ffi = require("ffi")
ffi.cdef[[
	// from  csr_typedef.h
	typedef char				i1;
	typedef unsigned char			u1;
	typedef short				i2;
	typedef unsigned short			u2;
	typedef int				i4;
	typedef unsigned int			u4;
	typedef long long			i8;
	typedef unsigned long long		u8;

	typedef struct TokenContext TokenContext;
	typedef struct TokenContextScript TokenContextScript;
	typedef struct LUAScript LUAScript;
	/*
	int strncmp ( const char * str1, const char * str2, size_t num );

    typedef struct _token_ctx {
        int a;
        char val[255];
    }token_ctx;
    //typedef int (*charlevel_callback_proto)(const char*, int);

    typedef int (__stdcall *charlevel_callback_proto)(token_ctx* ctx, const char*, int n);
    int must_call_callback(charlevel_callback_proto func, const char* msg);
	*/


	// call backs	  
    typedef int (__stdcall *char_prepare_cb)(TokenContext* ctx, TokenContextScript* script_ctx,
                                       u4 icode, u4 icode_lower, u2 icode_tag);
	
	// regist functions
	int reg_at_char_prepare(LUAScript* ctx, u4 icode, char_prepare_cb cb);

]]

function mm_init( ctx )
	local C = ffi.C
	local ctx_ptr = ffi.cast("LUAScript*", ctx)
	C.reg_at_char_prepare(ctx_ptr, 100, nil);
	print (ctx)
	return 0
end