﻿/*
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

#include "mm_api_script.h"

//extern "C" {
/*
 *  初始化 LUAScript 结构体
 */
int lua_script_init(LUAScript* ctx)
{
    ctx->L = luaL_newstate();
    luaL_openlibs(ctx->L);
    return 0;
}

int lua_script_clear(LUAScript* ctx)
{
    lua_close(ctx->L);
    return 0;
}

/*
 * 从文件中加载脚本
 *
 * return
 *  -1 脚本加载失败
 *  -2 脚本本身执行出错
 *  -3 脚本的注册函数执行错误
 */
int init_script(LUAScript* ctx, const char* script_fname)
{
    lua_State *L = ctx->L;

    int status = luaL_loadfile(L, script_fname);
    if (status) {
        // error loading
        return -1;
    }

    // execute the lua file. 让脚本可以初始化部分自身变量。
    status = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (status) {
        fprintf(stderr, "Failed to run script: %s\n", lua_tostring(L, -1));
        // error loading
        return -2;
    }

    // 调用 脚本的初始化函数，
    {
        double z;
        // Refer: http://lua-users.org/lists/lua-l/2011-06/msg00372.html
        /* push functions and arguments */
        lua_getglobal(L, "mm_init");  /* function to be called */
        lua_pushlightuserdata(L, &ctx);
        //lua_pushnumber(L, 10);   /* push 1st argument */

        /* do the call (1 arguments, 1 result) */
        if (lua_pcall(L, 1, 1, 0) != 0) {
            fprintf(stderr, "error running function `f': %s",
                 lua_tostring(L, -1));
            return -3;
        }

        /* retrieve result */
        if (!lua_isnumber(L, -1))
            printf("function `f' must return a number");
        z = lua_tonumber(L, -1);
        lua_pop(L, 1);  /* pop returned value */
        printf("get function 'f' result=%f\n", z);
    }

    return 0;
}

u2  get_dictionary_id_by_name(LUAScript* ctx, const char* dict_name)
{

    return 0;
}

/*
 * 注册字符级别的回调函数
 * 在分词过程启动前被调用, 当发现某个字的时候
 */
int reg_at_char_prepare(LUAScript* ctx, u4 icode, char_prepare_cb cb)
{
    return 0;
}

/*
 * 当某类字符类型
 */
int reg_at_chartag_prepare(LUAScript* ctx, u4 icode, char_prepare_cb cb)
{
    return 0;
}

/*
 * 注册单一字的回调函数
 * 分词之后 进行处理。例如 A  AB  ， 如果只处理 B ， 则不被激活
 */
int reg_at_char_post(LUAScript* ctx, u4 icode, char_cb cb)
{
    return 0;
}

/*
 * 当发现某个 term 时， 回调
 */
int reg_at_term(LUAScript* ctx, const char* term, u2 len, term_cb)
{
    return 0;
}

/*
 * 当发现来自某个词典的词条时， 如果这个词条同时出现在多个词典，也被此规则激活
 *
 * - 可以在脚本中，对具体的词条再进行判断
 */
int reg_at_dict(LUAScript* ctx, u2 dict_id, dict_cb)
{
    return 0;
}

/*
 * 注册回调到term的属性，当符合条件的属性
 *  -1 词典不存在
 *  -2 属性不存在
 *  -3 属性类型不匹配
 */
int reg_at_term_prop_u2(LUAScript* ctx, u2 dict_id, const char* prop, u2 v, dict_prop_u_cb)
{
    return 0;
}

int reg_at_term_prop_u4(LUAScript* ctx, u2 dict_id, const char* prop, u4 v, dict_prop_u_cb)
{
    return 0;
}

int reg_at_term_prop_u8(LUAScript* ctx, u2 dict_id, const char* prop, u8 v, dict_prop_u_cb)
{
    return 0;
}

int reg_at_term_prop_s(LUAScript* ctx, u2 dict_id, const char* prop, const char* sv, u2 sl,
                                    dict_prop_s_cb)
{
    return 0;
}


/* 数据处理回调有关, 被 LUA 的脚本中回调 */

/*
 * 得到当前字符在文档中的 offset
 */
u4 get_current_offset(TokenContext* ctx)
{
    return 0;
}

// 得到属性在词典中的编号
u2 get_dict_property_index(TokenContext* ctx, u2 dict_id, const char* prop)
{
    return 0;
}

u4 get_char(TokenContext* ctx, u2 idx)
{
    return 0;
}

//返回特定的位置
const char* get_term(TokenContext* ctx, u2 idx)
{
    return NULL;
}

/*
 *  在指定范围查找字符
 *  - 使用回调通知结果！！！ Great Idea！
 *  - 返回 ( TokenContextScript* script_ctx, iCode, pos, offset
 */
int find_char_in_range( TokenContext* ctx, TokenContextScript* script_ctx,
                      u4* icode, u2 icode_len, i2 begin, i2 end,  find_char_hit_cb cb)
{
    return 0;
}

// 查找多个词条
int fine_terms_in_range( TokenContext* ctx, TokenContextScript* script_ctx,
                       const char** term, u2* term_len, i2 begin, i2 end,
                       find_term_hit_cb cb )
{
    return 0;
}

// 根据词条的属性查找词条, 可能
int find_terms_by_property_u( TokenContext* ctx, TokenContextScript* script_ctx,
                             u2 dict_id, const char* prop, u8 v, i2 begin, i2 end,
                             find_term_hit_dict_cb cb )
{
    return 0;
}

// 根据词条的字符串属性查找词条
int find_terms_by_property_s( TokenContext* ctx, TokenContextScript* script_ctx,
                             u2 dict_id, const char* prop, const char* sv, u2 sl, i2 begin, i2 end,
                             find_term_hit_dict_cb cb )
{
    return 0;
}

// 根据词条的词典来源查找词条， 包括 OOV 词典， 用户词典
int find_term_by_dict( TokenContext* ctx, TokenContextScript* script_ctx,
                       u2 dict_id, i2 begin, i2 end,
                       find_term_hit_cb cb )
{
    return 0;
}

// 返回特定范围的原始字符串， utf8 格式
const char* get_string( TokenContext* ctx, i2 begin, i2 end )
{
    return NULL;
}

// 增加新的标引, idx 标引的位置， 标引的文字长度 (icode长度）
int add_annote_s(i2 idx, u2 len, u2 annote_type_id, const char* annote_data, u2 annote_data_len)
{
    return 0;
}

// 增加新标引， 如果目标的位数不足，将被截断
int add_annote_u(i2 idx, u2 len, u2 annote_type_id, u8 annote)
{
    return 0;
}

//} // end extern "C"
/* -- end of file -- */
