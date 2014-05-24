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

// FIXME: add cplusplus detected.

extern "C" {

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <stdlib.h>
#include <stdio.h>

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
 *  加载脚本时， LUA 可以使用的回调
 *
 *  Ref: http://luajit.org/ext_ffi_tutorial.html
 *
 *  本结构体由LUAJIT创建 （系统固化的脚本）
 *
 */
typedef struct LUAScript
{
    lua_State *L;
    //u4 balbalba;
    TokenContext* ctx; //当前执行的分词上下文。不可以持续绑定到 LUAScript

}LUAScript;

/*
 *  编码均使用 utf8
 */
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

typedef int (STDCALL *dict_prop_s_cb)(TokenContext* ctx, TokenContextScript* script_ctx,
                                       u2 dict_id, u4 entry_offset, const char* term, u2 term_len,
                                       const char* sv, const u2 sl);

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

u2  get_dictionary_id_by_name(LUAScript* ctx, const char* dict_name);

/*
 * 注册字符级别的回调函数
 * 在分词过程启动前被调用, 当发现某个字的时候
 */
int reg_at_char_prepare(LUAScript* ctx, u4 icode, char_prepare_cb cb); //, );

/*
 * 当某类字符类型
 */
int reg_at_chartag_prepare(LUAScript* ctx, u4 icode, char_prepare_cb cb);

/*
 * 注册单一字的回调函数
 * 分词之后 进行处理。例如 A  AB  ， 如果只处理 B ， 则不被激活
 */
int reg_at_char_post(LUAScript* ctx, u4 icode, char_cb cb);

/*
 * 当发现某个 term 时， 回调
 */
int reg_at_term(LUAScript* ctx, const char* term, u2 len, term_cb);

/*
 * 当发现来自某个词典的词条时， 如果这个词条同时出现在多个词典，也被此规则激活
 *
 * - 可以在脚本中，对具体的词条再进行判断
 */
int reg_at_dict(LUAScript* ctx, u2 dict_id, dict_cb);

/*
 * 注册回调到term的属性，当符合条件的属性
 *  -1 词典不存在
 *  -2 属性不存在
 *  -3 属性类型不匹配
 */
int reg_at_term_prop_u2(LUAScript* ctx, u2 dict_id, const char* prop, u2 v, dict_prop_u_cb);
int reg_at_term_prop_u4(LUAScript* ctx, u2 dict_id, const char* prop, u4 v, dict_prop_u_cb);
int reg_at_term_prop_u8(LUAScript* ctx, u2 dict_id, const char* prop, u8 v, dict_prop_u_cb);
int reg_at_term_prop_s(LUAScript* ctx, u2 dict_id, const char* prop, const char* sv, u2 sl,
                                    dict_prop_s_cb);


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
