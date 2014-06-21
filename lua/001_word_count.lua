# coding:utf8

local ffi = require("ffi")

-- print(mmseg_api)

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

]]

function mm_init( ctx )
	-- print ("in mm_init")
	local C = ffi.C
	local name_buf = ffi.new("char[?]", 4096)
	-- local mmseg_api = ffi.load("_mmseg.pyd")
	local ctx_ptr = ffi.cast("LUAScript*", ctx)
	local name_rs = C.get_dictionary_names(ctx_ptr, "term", name_buf, 4096)
	-- query each dictionary's id
	--for _, s in string.split(ffi.string(name_buf), ';') do
	-- for s in split(ffi.string(name_buf), ";") do
	if false then
		for s in string.gmatch(ffi.string(name_buf), '([^;]+)') do
		   -- print(s)
		   -- check  each dict's id
		   print(s, "id= ",C.get_dictionary_id_by_name(ctx_ptr, s) )
		end
		-- test get_dictionary_name
		if true then 
			C.get_dictionary_name(ctx_ptr, 4, name_buf, 4096)
			print("id=4, name=", ffi.string(name_buf))
		end
	end
	-- test lua's utf8 support
	if false then 
		local utf8_key = "中国"		-- can write term directly when source file encode in utf8. 
		print(utf8_key, utf8_key:len())
	end

	-- test regist rules
	local s_utf8_key = "中国"
	C.reg_at_term(ctx_ptr, 10, s_utf8_key, s_utf8_key:len(), false)	-- rule 10: hit when 中国 as a seg result
	C.reg_at_term(ctx_ptr, 20, s_utf8_key, s_utf8_key:len(), true)	-- rule 20: hit when 中国 appear in DAG ( term candicate )
	C.reg_at_term(ctx_ptr, 21, s_utf8_key, s_utf8_key:len(), true)	-- rule 20: hit when 中国 appear in DAG ( term candicate )
	C.reg_at_dict(ctx_ptr, 30, 5, false)  -- rule 30 dict_id = 5, the mmseg's base dict.
	-- "Zhong1 guo2"
	local s_prop_value = "Zhong1 guo2"  -- , test rule beyond 65536
	C.reg_at_term_prop_s(ctx_ptr, 80000, 4, "pinyin", s_prop_value, s_prop_value:len(), true) -- rule 80000, hit when a term's property is `Zhong1 guo2` in cedict(id=4), found in DAG.

	-- print ("out mm_init")
	-- mmseg_api.reg_at_char_prepare(ctx_ptr, 100, nil)  -- in mmseg_cli , ffi.load C.  should give a function load by exe or extension. ( python | java )
	--print (ctx)
	return 0
end