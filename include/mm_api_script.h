/*
 * Copyright 2014 Li Monan <limn@coreseek.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 */

#include <stdio.h>
#include "csr_typedefs.h"

#if defined(WIN32)
    #define STDCALL __stdcall
    #define LUAAPI   __declspec( dllexport )
#elif defined (__GNUG__)     /*  gcc  IIRC */
    #define STDCALL
    #define LUAAPI
#endif

/*
 * 本来设计为采用 C++ 回调 LUA 的方式， 根据
 *  Refer： http://luajit.org/ext_ffi_semantics.html#callback
 *
 * Callback 的性能会是严重的问题，修正为，每个 Segment 分析完成后， LUA 可以得到一次激活。
 *
 * 系统可根据 LUA 脚本预先的通知，标注出潜在的结果。  -> 条件判断与结果的缓存
 *
 */

// FIXME: add cplusplus detected.

extern "C" {

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <stdlib.h>
#include <stdio.h>

#define LUASCRIPT_ERROR_MESSAGE_LENGTH  1024u
#define LUASCRIPT_CALLBACK_BLOCK_SIZE   16

#define LUASCRIPT_STATUS_INIT           1u
#define LUASCRIPT_STATUS_EXEC           2u

// typedef void* SegScriptPtr; // 一个指向
/*
 *  分词使用的上下文，通过本结构体 & 其中的指针，可以 操作 SegStatus ，并修改结果
 */
typedef struct TokenContext
{
    u4 id;
}TokenContext;

/*
 * 被脚本使用的分词上下文 ? 用于在主持回调时，传递上下文信息，
 * 不适用
 */
typedef struct TokenContextScript
{
    u4 obj_id;
}TokenContextScript;

/*
 *  编码均使用 utf8
 */

//typedef int (STDCALL *script_init_cb)(LUAScript* script);

typedef int (STDCALL *char_prepare_cb)(TokenContext* ctx, TokenContextScript* script_ctx,
                                       u4 icode, u4 icode_lower, u2 icode_tag);

typedef int (STDCALL *char_cb)(TokenContext* ctx, TokenContextScript* script_ctx,
                                       u4 icode, u2 icode_tag);

typedef int (STDCALL *term_cb)(TokenContext* ctx, TokenContextScript* script_ctx,
                                       const char* term, u2 term_len );

typedef int (STDCALL *dict_cb)(TokenContext* ctx, TokenContextScript* script_ctx,
                                       u2 dict_id, u4 entry_offset, const char* term, const u2 term_len );

typedef int (STDCALL *dict_prop_u_cb)(TokenContext* ctx, TokenContextScript* script_ctx,
                                       u2 dict_id, u4 entry_offset, const char* term, const u2 term_len,
                                       const u8 v);

// 注册某个属性的值，值的长度不能超过 255
typedef int (STDCALL *dict_prop_s_cb)(TokenContext* ctx, TokenContextScript* script_ctx,
                                       u2 dict_id, u4 entry_offset, const char* term, u2 term_len,
                                       const char* sv, const u2 sl);

typedef struct _LUAScriptCallBackEntryMeta
{
    u1 type; // c = char prepare; C =char; t=term; d = dict; D=dictwithprop; S=dictwithprops.
    union _d {
        u4 icode;       // 用于基于字的回调
        struct {
            u2 dict_id;
            u2 prop_id;
            u8 prop_v;  // hash of prop. 这样可以通过统一的二分查找表处理 字典属性的问题。
        }dict_p;
        u2 dict_id; // 处理字典的问题
    };
}_LUAScriptCallBackEntryMeta;

//基于字的回调， 使用二分查找表。
typedef struct LUAScriptCallBackEntry
{
    _LUAScriptCallBackEntryMeta meta;
    void* cb;   // can be char_prepare_cb | char_cb | term_cb | dict_cb | dict_prop_u_cb | dict_prop_s_cb

}LUAScriptCallBackEntry;

// dict_id prop_id, prop_value
// dict_id
// entry_offset @globalidx? , 隐含的， 这个词必须在词表中。 使用二分查找

typedef struct LUAScriptCallBackList
{

    LUAScriptCallBackEntry callbacks[LUASCRIPT_CALLBACK_BLOCK_SIZE];
    LUAScriptCallBackList* next;
}LUAScriptCallBackList;


/*
 *  加载脚本时， LUA 可以使用的回调
 *
 *  Ref: http://luajit.org/ext_ffi_tutorial.html
 *
 *  本结构体由LUAJIT创建 （系统固化的脚本）
 *
 *  全部初始化完成后， 会调用 warmup ， 执行 "hello world."
 */
typedef struct LUAScript
{
    u1 stage;          // 加载脚本 | 加载完成
    lua_State *L;
    //u4 balbalba;
    TokenContext* ctx; //当前执行的分词上下文。不可以持续绑定到 LUAScript
    char error_msg[LUASCRIPT_ERROR_MESSAGE_LENGTH];       // the errror message of lua script.
    LUAScriptCallBackList registed_cb;  // 用于保存 LUA 脚本传递来的回调， 需要 build_cb_index 来构造索引， 一旦构造完成， 就不可以增加新的 cb 了
}LUAScript;


/*
 * 用于返回查询数据的 回调
 * - return 0 if continue
 * - return < 0 if no needs further look.
 */
typedef int (STDCALL * find_char_hit_cb) ( TokenContextScript* script_ctx, u4 icode,
                                           i2 pos, u4 offset);

typedef int (STDCALL * find_term_hit_cb) ( TokenContextScript* script_ctx, const char* term,
                                           u2 term_len, u4 offset);

// 此处不再给 属性信息，如果按照属性，需要独立的注册回调（每种情况一个）
typedef int (STDCALL * find_term_hit_dict_cb) ( TokenContextScript* script_ctx, const char* term,
                                                u2 term_len, u4 offset,
                                                u2 dict_id, u4 entry_offset );

/* 系统初始化有关 */
LUAAPI int lua_script_init(LUAScript* ctx);

// 清除全部 结构体包括的指针
LUAAPI int lua_script_clear(LUAScript* ctx);

/*
 * 从文件中加载脚本
 */
LUAAPI int init_script(LUAScript* ctx, const char* script_fname);  // called c side

LUAAPI u2  get_dictionary_id_by_name(LUAScript* ctx, const char* dict_name);

/*
 * 注册字符级别的回调函数
 * 在分词过程启动前被调用, 当发现某个字的时候
 */
LUAAPI int reg_at_char_prepare(LUAScript* ctx, u4 icode, char_prepare_cb cb); //, );

/*
 * 当某类字符类型
 */
LUAAPI int reg_at_chartag_prepare(LUAScript* ctx, u4 icode, char_prepare_cb cb);

/*
 * 注册单一字的回调函数
 * 分词之后 进行处理。例如 A  AB  ， 如果只处理 B ， 则不被激活
 */
LUAAPI int reg_at_char_post(LUAScript* ctx, u4 icode, char_cb cb);

/*
 * 当发现某个 term 时， 回调
 */
LUAAPI int reg_at_term(LUAScript* ctx, const char* term, u2 len, term_cb);

/*
 * 当发现来自某个词典的词条时， 如果这个词条同时出现在多个词典，也被此规则激活
 *
 * - 可以在脚本中，对具体的词条再进行判断
 */
LUAAPI int reg_at_dict(LUAScript* ctx, u2 dict_id, dict_cb);

/*
 * 注册回调到term的属性，当符合条件的属性
 *  -1 词典不存在
 *  -2 属性不存在
 *  -3 属性类型不匹配
 */
LUAAPI int reg_at_term_prop_u2(LUAScript* ctx, u2 dict_id, const char* prop, u2 v, dict_prop_u_cb);
LUAAPI int reg_at_term_prop_u4(LUAScript* ctx, u2 dict_id, const char* prop, u4 v, dict_prop_u_cb);
LUAAPI int reg_at_term_prop_u8(LUAScript* ctx, u2 dict_id, const char* prop, u8 v, dict_prop_u_cb);
LUAAPI int reg_at_term_prop_s(LUAScript* ctx, u2 dict_id, const char* prop, const char* sv, u2 sl,
                                    dict_prop_s_cb);


int build_cb_index(LUAScript* ctx);

/* 数据处理回调有关, 被 LUA 的脚本中回调 */

/*
 * 得到当前字符在文档中的 offset
 */
u4 get_current_offset(TokenContext* ctx);

// 得到属性在词典中的编号
u2 get_dict_property_index(TokenContext* ctx, u2 dict_id, const char* prop);

u4 get_char(TokenContext* ctx, u2 idx);
const char* get_term(TokenContext* ctx, u2 idx); //返回特定的位置

/*
 *  在指定范围查找字符
 *  - 使用回调通知结果！！！ Great Idea！
 *  - 返回 ( TokenContextScript* script_ctx, iCode, pos, offset
 */
int find_char_in_range( TokenContext* ctx, TokenContextScript* script_ctx,
                      u4* icode, u2 icode_len, i2 begin, i2 end,  find_char_hit_cb cb);

// 查找多个词条
int fine_terms_in_range( TokenContext* ctx, TokenContextScript* script_ctx,
                       const char** term, u2* term_len, i2 begin, i2 end,
                       find_term_hit_cb cb );

// 根据词条的属性查找词条, 可能
int find_terms_by_property_u( TokenContext* ctx, TokenContextScript* script_ctx,
                             u2 dict_id, const char* prop, u8 v, i2 begin, i2 end,
                             find_term_hit_dict_cb cb );

// 根据词条的字符串属性查找词条
int find_terms_by_property_s( TokenContext* ctx, TokenContextScript* script_ctx,
                             u2 dict_id, const char* prop, const char* sv, u2 sl, i2 begin, i2 end,
                             find_term_hit_dict_cb cb );

// 根据词条的词典来源查找词条， 包括 OOV 词典， 用户词典
int find_term_by_dict( TokenContext* ctx, TokenContextScript* script_ctx,
                       u2 dict_id, i2 begin, i2 end,
                       find_term_hit_cb cb );

// 返回特定范围的原始字符串， utf8 格式
const char* get_string( TokenContext* ctx, i2 begin, i2 end );

// 增加新的标引, idx 标引的位置， 标引的文字长度 (icode长度）
int add_annote_s(i2 idx, u2 len, u2 annote_type_id, const char* annote_data, u2 annote_data_len);
// 增加新标引， 如果目标的位数不足，将被截断
int add_annote_u(i2 idx, u2 len, u2 annote_type_id, u8 annote);


} // end extern "C"
