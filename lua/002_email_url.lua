# coding:utf8

local ffi = require("ffi")

-- 处理 Email & URL ， 对不同的部分进行特别合并

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
	typedef struct LUAScript LUAScript;
	
	int get_dictionary_names(LUAScript* ctx, const char* category, char* data_ptr, int data_len);
	int get_dictionary_name(LUAScript* ctx, int dict_id, char* data_ptr, int data_len);
	u2 get_dictionary_id_by_name(LUAScript* ctx, const char* dict_name);

	int reg_at_term(LUAScript* ctx, int rule_id, const char* term, u2 term_len, bool bInDAG);
	int reg_at_dict(LUAScript* ctx, int rule_id, u2 dict_id, bool bInDAG);

	int reg_at_term_prop_u2(LUAScript* ctx, int rule_id, u2 dict_id, const char* prop, u2 v, bool bInDAG);
	int reg_at_term_prop_u4(LUAScript* ctx, int rule_id, u2 dict_id, const char* prop, u4 v, bool bInDAG);
	int reg_at_term_prop_u8(LUAScript* ctx, int rule_id, u2 dict_id, const char* prop, u8 v, bool bInDAG);
	int reg_at_term_prop_s(LUAScript* ctx, int rule_id, u2 dict_id, const char* prop, const char* sv, u2 sl, bool bInDAG);

        typedef int (__stdcall *script_processor_proto)(LUAScript* ctx, int start_pos);

        int reg_proc(LUAScript* ctx, script_processor_proto proc);
]]

function mm_init( ctx )
    -- print ("in mm_init")
    local C = ffi.C
    local ctx_ptr = ffi.cast("LUAScript*", ctx)

    local s_utf8_key = "@" -- 当出现 @ 符号时... 可能是邮件，可能是微博
    local n = C.reg_at_term(ctx_ptr, 500, s_utf8_key, s_utf8_key:len(), false)
    print(n)
    -- print("mm_init in email")  -- 用于测试函数是否会冲突; 不会冲突
    local process_cb = ffi.cast("script_processor_proto",
        function(ctx, start_pos)
            print("hello")
            return 0
        end )

    C.reg_proc(ctx_ptr, process_cb)
    -- FIXME: should release the cb
    -- process_cb:free()
    return 0
end
