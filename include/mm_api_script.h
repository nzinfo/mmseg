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

#if !defined(_LUA_SCRIPT_API_H)
#define _LUA_SCRIPT_API_H

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
 * 需要处理的时机：
 *
 *  2 出现某个候选Term（在DAG中）, 因为可以把单字作为词条加入词典中，不额外提供单字的接口
 *  3 出现某个短语（已经在结果中）
 *  4 出现某个词典中的词（在结果中|在DAG中）
 *  5 出现某个词典中的某个属性为 S1 的词（在DAG中）
 *  6 出现某个词典中的某个属性为 S1 的词（在结果中）
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

/*
 *  分词使用的上下文，通过本结构体 & 其中的指针，可以 操作 SegStatus ，并修改结果
 */
typedef struct TokenContext
{
    u4 id; // 我忘记做什么， 用的了
    void* seg_status_ptr;   // 指向当前 SegStatus 的指针
}TokenContext;


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
    int script_id;      // 用于判断当前执行的那个脚本。 0 表示系统初始化完成，处于数据处理阶段
    TokenContext* task_ctx; //当前执行的分词上下文。不可以持续绑定到 LUAScript
    char error_msg[LUASCRIPT_ERROR_MESSAGE_LENGTH];       // the errror message of lua script.
    void* seg_script_ptr;
}LUAScript;

/* 系统初始化有关 */
LUAAPI
int lua_script_init(LUAScript* ctx);

// 清除全部 结构体包括的指针
LUAAPI
int lua_script_clear(LUAScript* ctx);

/*
 * 从文件中加载脚本
 */
LUAAPI
int init_script(LUAScript* ctx, int script_id, const char* script_fname);  // called c side

LUAAPI
int init_script_done(LUAScript* ctx);        // 用户脚本已经全部加载完毕。

// 向系统中查询与词典相关的信息

LUAAPI
int get_dictionary_names(LUAScript* ctx, const char* category, char* data_ptr, int data_len);
LUAAPI
int get_dictionary_name(LUAScript* ctx, int dict_id, char* data_ptr, int data_len);
LUAAPI
u2 get_dictionary_id_by_name(LUAScript* ctx, const char* dict_name);


/*
 * 向系统中注册命中规则
 */

/*
 * 当发现某个 term 时， 回调
 *
 * @rule_id 本规则的变化，用于区分命中的结果；
 * @term 关注的词
 * @bInDAG == true, 出现在候选的词表中即激活该规则
 *
 * Note:
 *  花了很长时间考虑如何处理词共现关系，可以考虑只注册词的关系，然后从命中结果中·分析·出词共现关系。
 *  这样灵活性更高，而且易于实现。
 *
 * 维护一个列表，记录当命中该规则时，需要激活的 Rule （额外记录Term的长度， 用于辅助开发）
 * 修改 全局的 Index， 内部存储一个 ID， 在全局 Index 中记录这个 ID
 *
 * MatchEntry  - {seg|dag} - innerID -> list of Rules. (因为同一个词条，可能命中多个规则)
 * 并不是每个规则都需要直接命中的，对于 dict 类 的 Rule ，还需要额外检查 dict & property.
 *
 */
LUAAPI
int reg_at_term(LUAScript* ctx, int rule_id, const char* term, u2 term_len, bool bInDAG);

/*
 * 当候选的词中出现来自某个词典的词时。
 */
LUAAPI
int reg_at_dict(LUAScript* ctx, int rule_id, u2 dict_id, bool bInDAG);

/*
 * 注册回调到term的属性，当符合条件的属性
 *  -1 词典不存在
 *  -2 属性不存在
 *  -3 属性类型不匹配
 */
LUAAPI
int reg_at_term_prop_u2(LUAScript* ctx, int rule_id, u2 dict_id, const char* prop, u2 v, bool bInDAG);
LUAAPI
int reg_at_term_prop_u4(LUAScript* ctx, int rule_id, u2 dict_id, const char* prop, u4 v, bool bInDAG);
LUAAPI
int reg_at_term_prop_u8(LUAScript* ctx, int rule_id, u2 dict_id, const char* prop, u8 v, bool bInDAG);
LUAAPI
int reg_at_term_prop_s(LUAScript* ctx, int rule_id, u2 dict_id, const char* prop,
                       const char* sv, u2 sl, bool bInDAG);

// 当一个Block被填满，或者最后一个 Block。 回调本函数
typedef int (STDCALL *script_processor_proto)(LUAScript* ctx, int start_pos);

LUAAPI
int reg_proc(LUAScript* ctx, script_processor_proto proc);

/* 数据处理回调有关, 被 LUA 的脚本中回调 */

/*
 * 得到当前字符在文档中的 offset
 */
u4 get_current_offset(TokenContext* ctx);

// 得到属性在词典中的编号
u2 get_dict_property_index(TokenContext* ctx, u2 dict_id, const char* prop);

u4 get_char(TokenContext* ctx, u2 idx);
const char* get_term(TokenContext* ctx, u2 idx); //返回特定的位置

#if 0
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

#endif

// 返回特定范围的原始字符串， utf8 格式
const char* get_string( TokenContext* ctx, i2 begin, i2 end );

// 增加新的标引, idx 标引的位置， 标引的文字长度 (icode长度）
int add_annote_s(i2 idx, u2 len, u2 annote_type_id, const char* annote_data, u2 annote_data_len);
// 增加新标引， 如果目标的位数不足，将被截断
int add_annote_u(i2 idx, u2 len, u2 annote_type_id, u8 annote);


} // end extern "C"

#endif //_LUA_SCRIPT_API_H
