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


#if !defined(_SEGSCRIPT_H)
#define _SEGSCRIPT_H

#include "mm_segpolicy.h"
#include "mm_api_script.h"
#include "mm_stringpool.h"

namespace mm {

/*
 * 实际为 Script Manager， 从指定目录加载脚本文件
 *
 * - 暴露 API 给脚本
 * - 本对象全局唯一
 *
 *
 */
//typedef u1* rule_list; //使用文本形式编码(big utf-8)过的rules

class DictMgr;


typedef union prop_v {
    u2 v_u2;
    u4 v_u4;
    u8 v_u8;
}prop_v;

class SegScriptRule {

    // 用于存储用户注册的规则
public:
    char        rule_type; // t=term; d=dict; 2=u2; 4=u4; 8=u8; s=string
    int         rule_id;
    int         script_id;
    u2          dict_id;
    std::string term;
    i4          term_entry_offset;
    int         prop_idx;
    prop_v      v;
    std::string v_str; // 当 type=s 时，需要读取的值
    bool        in_dag;
};

class SegScriptProc {
public:
    int script_id;
    script_processor_proto proc;
};

typedef std::vector<SegScriptRule> SegScriptRuleList;
typedef std::vector<SegScriptProc> SegScriptProcList;

class SegScript {
public:
    SegScript(DictMgr* dict_mgr);
    virtual ~SegScript();

public:
    //SegScriptPeer _peer;
    /*
     * load script from disk.
     * load order by ascii orer
     */
    int LoadScripts(const std::string script_path);
    const std::string & GetErrorMessage() const {
        return _err_msg;
    }

public: // used @lua side
    int RegTerm(int rule_id, int script_id, const char* term, u2 term_len, bool bInDAG);
    int RegDict(int rule_id, int script_id, u2 dict_id, bool bInDAG);
    int RegPropU2(int rule_id, int script_id, u2 dict_id, const char* prop, u2 v, bool bInDAG);
    int RegPropU4(int rule_id, int script_id, u2 dict_id, const char* prop, u4 v, bool bInDAG);
    int RegPropU8(int rule_id, int script_id, u2 dict_id, const char* prop, u8 v, bool bInDAG);
    int RegPropStr(int rule_id, int script_id, u2 dict_id, const char* prop,
                           const char* sv, u2 sl, bool bInDAG);

    int RegProc(int script_id, script_processor_proto proc);

    int RemoveRulesByScriptId(int script_id);
    int RemoveProcessorCallBack(int script_id);

    int BuildRegIndex();

    const char* GetDictionaryName(u2 dict_id) const;
    const std::string GetDictionaryNames(const char* category) const;
    int   GetDictionaryID(const char* dict_name) const;

protected:
    // find is term in global_idx

protected:
    DictMgr*   _dict_mgr;
    LUAScript* _script;
    std::string _err_msg;

    // 与 script 的 hook 有关，保存激活规则的 RuleID
    //std::vector<rule_list> _dict_segtag2rulelist; //需要在初始化时，预先 push 一个元素，以避免属性的偏移量是0
    //std::vector<rule_list> _dict_dagtag2rulelist;

    SegScriptRuleList  _rules;
    SegScriptProcList  _procs;
    mm::StringPoolMemory _rulehits_pool;    //保存字符串形式的规则的命中， 自动去重。

protected:
    void _KeepAPICode();// 进行API的无实际用途的调用，以保证代码不会被优化掉。
};

} // namespace mm

//////////////////////////////////////////////////////////////////////////////////////////

#endif  //_SEGSCRIPT_H
