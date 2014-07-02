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

#include <glog/logging.h>
#include <sstream>
#include <algorithm>

#include "mm_seg_script.h"
#include "utils/pystring.h"
#include "mm_dict_mgr.h"
#include "utils/utf8_to_16.h"

namespace mm {

SegScript::SegScript(DictMgr *dict_mgr)
    :_dict_mgr(dict_mgr)
{
    _script = new LUAScript();
    _script->seg_script_ptr = this;
    lua_script_init(_script);

    // 初始化 rules
    _rules.reserve(1024);
    _rulehits_pool.AllocString("hello", 5); // advance offset make it not zero.
}

SegScript::~SegScript() {
    lua_script_clear(_script);

    delete _script;
}

int SegScript::LoadScripts(const std::string script_path)
{
    /*
     *  1 get all file in the path
     *  2 order it in asc
     *  3 load
     */
    std::vector<std::string> lua_scripts;
    // if path have more dict. only the first 20 , 24 solt, 0~3 reversed.
    int nfiles = mm::DictMgr::GetDictFileNames(script_path.c_str(), ".lua", true, lua_scripts);
    // order
    std::sort(lua_scripts.begin(),lua_scripts.end());
    // load each
    int rs = 0;
    std::string script_fname;
    int script_id = 0;
    for(std::vector<std::string>::iterator it = lua_scripts.begin();
         it != lua_scripts.end(); ++it) {
#ifndef WIN32
             script_fname = pystring::os::path::join(script_path, *it);
#else
             script_fname = pystring::os::path::join_nt(script_path, *it);
#endif // !WIN32
        rs = init_script(_script,  script_id, script_fname.c_str());  //FIXME: how to report error ?
        if(rs < 0) {
            // FIXME: addtional error from _script
            _err_msg = "error execute " + *it + _script->error_msg;
            return rs;
        }
        LOG(INFO) << "load script " << *it;
        script_id ++;
    }
    init_script_done(_script);
    return nfiles;
}

int SegScript::RegTerm(int rule_id, int script_id, const char* term, u2 term_len, bool bInDAG)
{
    if(rule_id == 0) return -1; // 0 is string's end marker, will make encode/decode puzzle
    /*
     * 1 find the term
     * 2 check seg | dag is 0
     * 3 if true add a new list to ...
     * 4 if false, add rule_id to list.
     */

    mm::DictBase* idx = _dict_mgr->GetGlobalTermIndex();
    int v = idx->ExactMatch(term, term_len);
    if(v<0)
        return -1; // term not found.

    SegScriptRule rule;
    rule.script_id = script_id;
    rule.dict_id = 0;
    rule.in_dag = bInDAG;
    rule.rule_id = rule_id;
    rule.term = term;   // ignore term_len now.
    rule.term_entry_offset = v;
    rule.rule_type = 't';
    _rules.push_back(rule);
    return 0;
}

int SegScript::RegDict(int rule_id, int script_id, u2 dict_id, bool bInDAG){
    /*
     *  add to each item in dict ?
     *  1 check dic_id exist
     *
     */
    if(rule_id == 0) return -1;
    SegScriptRule rule;
    rule.script_id = script_id;
    rule.dict_id = dict_id;
    rule.in_dag = bInDAG;
    rule.rule_id = rule_id;
    rule.rule_type = 'd';
    _rules.push_back(rule);

    return 0;
}

int SegScript::RegPropU2(int rule_id, int script_id, u2 dict_id, const char* prop, u2 v, bool bInDAG)
{
    /*
     * add to each item meet require props?
     * - 如果有词条同时命中多个规则如何？
     * # 如果存在一个可能包括全部词条的词典？在 Global Idx 中，实际注册的是查询的结果。
     * # 对于同一个词，DAG 与 Seg 对应两个结果。
     *
     * 1 check dict existance
     * 2 check prop existance
     * - if no item's property == v, the rule still there.
     *
     */
    // FIXME: should check prop's existance & type.
    if(rule_id == 0) return -1;
    mm::DictBase* dict_ptr = _dict_mgr->GetDictionary(dict_id);
    if(!dict_ptr)   return -2;
    const mm::DictSchemaColumn* colmn = dict_ptr->GetSchema()->GetColumn(prop);
    if(!colmn)      return -3; //no such column
    if(colmn->GetType()!= '2')  return -4; //column type mismatch.

    SegScriptRule rule;
    rule.script_id = script_id;
    rule.dict_id = dict_id;
    rule.in_dag = bInDAG;
    rule.rule_id = rule_id;
    rule.prop_idx = colmn->GetIndex();
    rule.v.v_u2 = v;
    rule.rule_type = '2';
    _rules.push_back(rule);

    return 0;
}

int SegScript::RegPropU4(int rule_id, int script_id, u2 dict_id, const char* prop, u4 v, bool bInDAG)
{
    if(rule_id == 0) return -1;
    mm::DictBase* dict_ptr = _dict_mgr->GetDictionary(dict_id);
    if(!dict_ptr)   return -2;
    const mm::DictSchemaColumn* colmn = dict_ptr->GetSchema()->GetColumn(prop);
    if(!colmn)      return -3; //no such column
    if(colmn->GetType()!= '4')  return -4; //column type mismatch.

    SegScriptRule rule;
    rule.script_id = script_id;
    rule.dict_id = dict_id;
    rule.in_dag = bInDAG;
    rule.rule_id = rule_id;
    rule.prop_idx = colmn->GetIndex();
    rule.v.v_u4 = v;
    rule.rule_type = '4';
    _rules.push_back(rule);

    return 0;
}

int SegScript::RegPropU8(int rule_id, int script_id, u2 dict_id, const char* prop, u8 v, bool bInDAG)
{
    if(rule_id == 0) return -1;
    mm::DictBase* dict_ptr = _dict_mgr->GetDictionary(dict_id);
    if(!dict_ptr)   return -2;
    const mm::DictSchemaColumn* colmn = dict_ptr->GetSchema()->GetColumn(prop);
    if(!colmn)      return -3; //no such column
    if(colmn->GetType()!= '8')  return -4; //column type mismatch.

    SegScriptRule rule;
    rule.script_id = script_id;
    rule.dict_id = dict_id;
    rule.in_dag = bInDAG;
    rule.rule_id = rule_id;
    rule.prop_idx = colmn->GetIndex();
    rule.v.v_u8 = v;
    rule.rule_type = '8';
    _rules.push_back(rule);

    return 0;
}

int SegScript::RegPropStr(int rule_id, int script_id, u2 dict_id, const char* prop,
                       const char* sv, u2 sl, bool bInDAG)
{
    if(rule_id == 0) return -1;
    mm::DictBase* dict_ptr = _dict_mgr->GetDictionary(dict_id);
    if(!dict_ptr)   return -2;
    const mm::DictSchemaColumn* colmn = dict_ptr->GetSchema()->GetColumn(prop);
    if(!colmn)      return -3; //no such column
    if(colmn->GetType()!= 's')  return -4; //column type mismatch.

    SegScriptRule rule;
    rule.script_id = script_id;
    rule.dict_id = dict_id;
    rule.in_dag = bInDAG;
    rule.rule_id = rule_id;
    rule.prop_idx = colmn->GetIndex();
    rule.v_str = std::string(sv, sl);
    rule.rule_type = 's';
    _rules.push_back(rule);
    return 0;
}

int SegScript::RegProc(int script_id, script_processor_proto proc)
{
    SegScriptProc seg_proc;
    seg_proc.script_id = script_id;
    seg_proc.proc  = proc;
    _procs.push_back(seg_proc);
    return 0;
}

bool rule_script_id_cmp(SegScriptRule& r)
{
    return r.script_id == -1;
}

bool proc_script_id_cmp(SegScriptProc& r)
{
    return r.script_id == -1;
}

int SegScript::RemoveRulesByScriptId(int script_id)
{
    /*
     * 删除某个 script_id 注册的全部 rule； rule_id 必须是全局唯一的
     */
    SegScriptRuleList::iterator it;
    for(it = _rules.begin(); it < _rules.end(); it++) {
        if( (*it).script_id == script_id)
            (*it).script_id = -1;
    }

    it = std::remove_if(_rules.begin(), _rules.end(),
                        rule_script_id_cmp);
    _rules.erase(it);
    return 0;
}

int SegScript::RemoveProcessorCallBack(int script_id)
{
    SegScriptProcList::iterator it;
    for(it = _procs.begin(); it < _procs.end(); it++) {
        if( (*it).script_id == script_id)
            (*it).script_id = -1;
    }

    it = std::remove_if(_procs.begin(), _procs.end(),
                        proc_script_id_cmp);
    _procs.erase(it);

    return 0;
}

int SegScript::BuildRegIndex()
{
    /*
     * 用于在全部完成后，构造 Prop & Term 的快速查找表。
     *
     * - 需要给 Dict 一个遍历全部 Term 的能力..., 至少是全部属性条目的能力
     * - 在全局索引中，可以根据 entries 属性，得到某个词典中是否含有这个词。
     *
     * 1 遍历全部 term，对同一个 term 下的多个 rule 进行合并；
     * 2 在 global_idx 中查询 term， 设置对应的 dag & seg; 使用 utf-8 编码
     *
     * ==========
     * 1 遍历全部 globalidx 的 data，检查与 rule 的匹配程度，构造 match_utf8_str;
     *
     * 3 检查现有的 seg & dag 对应 的 utf-8值，如果存在，需要 两个值连接
     * 4 保存到 string_pool
     * 5 返回 pointer ， 计算 pointer 到起始位置的 offset， 得到 seg & dag 的值
     *
     */
    // 保存基于 Term 的 RuleHit List
    unordered_map<i4, std::string> term_hit_string_seg;
    unordered_map<i4, std::string> term_hit_string_dag;
    unordered_map<i4, std::string>::iterator term_it;

    // 保存基于 dict_id 的规则, 且不包括按照词条命中的。理论上应该不多了。
    SegScriptRuleList rules_by_dictid[TOTAL_DICTIONARY_COUNT];
    // FIXME:  how about custom dict, as regist rule needs dict id so...
    mm::DictBase*     dicts[TOTAL_DICTIONARY_COUNT];    // all the system based dict.
    memset(dicts, 0, sizeof(dicts));

    // read dag | seg prop's idx in global_idx
    mm::DictBase* idx = _dict_mgr->GetGlobalTermIndex();
    const DictSchemaColumn* colmn = idx->GetSchema()->GetColumn("dag");
    if(!colmn)
        return -1; // dag | seg not define.
    short dag_idx = colmn->GetIndex();

    colmn = idx->GetSchema()->GetColumn("seg");
    if(!colmn)
        return -1; // dag | seg not define.
    short seg_idx = colmn->GetIndex();

    colmn = idx->GetSchema()->GetColumn("entries");
    if(!colmn)
        return -1; // entries not define.
    short entries_idx = colmn->GetIndex();

    char utf8_hit[8];
    for(SegScriptRuleList::const_iterator it = _rules.begin();
        it < _rules.end(); it++) {
        if( (*it).rule_type == 't' ) {
            /*
             * 1 check is in term_hit_string, if true, append new ruleid
             * 2 if not set ...
             */
            int utf8_hit_len = csr::csrUTF8Encode((u1*)utf8_hit, (*it).rule_id);
            utf8_hit[utf8_hit_len] = 0;
            term_it = term_hit_string_seg.find((*it).term_entry_offset);
            if(term_it == term_hit_string_seg.end()) {
                // add new hit string
                term_hit_string_seg[(*it).term_entry_offset] = std::string(utf8_hit);
                term_hit_string_dag[(*it).term_entry_offset] = std::string(utf8_hit);
            } else{
                // append hit string
                term_hit_string_seg[(*it).term_entry_offset] += std::string(utf8_hit);
                term_hit_string_dag[(*it).term_entry_offset] += std::string(utf8_hit);
            }
        } else {
            // 除了 Term 之外，都需要依赖 dict_id
            rules_by_dictid[(*it).dict_id].push_back(*it);
        }
    } // end for terms.

    // foreach entry_data in globalidx
    // 潜在的要求： global index 的 entrydata 是不能被压缩的。
    u1 buf[4096]; // 因为是 extactlymatch， buffer 尺寸应该够了。
    mm::DictMatchResult rs(buf, 4096/sizeof(mm::DictMatchEntry));
    u2 data_len;

    u1 dict_seg_hit[2048]; // 32 dict,
    u1 dict_dag_hit[2048]; // hit miggt full, if full just ignore.
    u1 *dict_seg_hit_ptr = dict_seg_hit;
    u1 *dict_seg_hit_end = dict_seg_hit + 2040; // leave high 8byte empty.
    u1 *dict_dag_hit_ptr = dict_dag_hit;
    u1 *dict_dag_hit_end = dict_dag_hit + 2040;

    u2 entry_size = idx->GetSchema()->GetEntryDataSize();
    mm::EntryData* entry_data_ptr = NULL;
    mm::EntryData* entry_pre_dict_data_ptr = NULL;
    bool rule_hit = false;
    for(u4 i=0; i< idx->EntryCount(); i ++) {
        data_len = 0;
        rs.Reset();

        dict_seg_hit_ptr = dict_seg_hit;
        dict_dag_hit_ptr = dict_dag_hit;

        entry_data_ptr = idx->GetEntryDataByOffset(i*entry_size);
        // test dump the entry per dict hit list.
        const char* sptr = (const char*)entry_data_ptr->GetData(idx->GetSchema(),
            idx->GetStringPool(), entries_idx, &data_len);

        if(sptr == NULL) {
            // strange.. 因为有 ucs-2 字符不全的原因，为 NULL 的常见。
            printf("i=%d, offset=%d\n", i, i*entry_size);
            continue;
        }
        // give term_len always 1, don't care about term_len
        if(sptr)
        {
            int rs_n = decode_entry_to_matchentry((const u1*)sptr, data_len, 1, &rs);
            for(int j = 0; j<rs_n;j++) {
                //printf("@offset %d, dict id=%d, offset in sub dict=%d\n", i*entry_size, rs.GetMatch(j)->match._dict_id, rs.GetMatch(j)->match._value);
                // loop for each dict's match.
                u2   dict_id = rs.GetMatch(j)->match._dict_id;
                // check props.
                SegScriptRuleList& rule_list = rules_by_dictid[dict_id];
                // try fetch dict_id
                if(dicts[dict_id] == NULL)
                {
                    dicts[dict_id] = _dict_mgr->GetDictionary(dict_id);
                    if(!dicts[dict_id])
                        continue; //BAD, no such dictionary. should find it early.
                }
                for(SegScriptRuleList::iterator rule_it = rule_list.begin();
                    rule_it < rule_list.end(); rule_it++) {
                    rule_hit = false;
                    // ignore any new hit if full
                    if((*rule_it).rule_type == 'd'){
                        rule_hit = true;
                    }else{
                        // not whole dict, must need check property's value.
                        entry_pre_dict_data_ptr = dicts[dict_id]->GetEntryDataByOffset(rs.GetMatch(j)->match._value);
                    }
                    // check property.
                    if((*rule_it).rule_type == '2' && entry_pre_dict_data_ptr){
                        // hit when this dict appear.
                        if( (*rule_it).v.v_u2 == entry_pre_dict_data_ptr->GetU2(dicts[dict_id]->GetSchema(), (*rule_it).prop_idx, 0 ) )
                        {
                            rule_hit = true;
                        }
                    } // end type = 2
                    if((*rule_it).rule_type == '4' && entry_pre_dict_data_ptr){
                        // hit when this dict appear.
                        if( (*rule_it).v.v_u4 == entry_pre_dict_data_ptr->GetU4(dicts[dict_id]->GetSchema(), (*rule_it).prop_idx, 0 ) )
                        {
                            rule_hit = true;
                        }
                    } // end type = 4
                    if((*rule_it).rule_type == '8' && entry_pre_dict_data_ptr){
                        // hit when this dict appear.
                        if( (*rule_it).v.v_u8 == entry_pre_dict_data_ptr->GetU8(dicts[dict_id]->GetSchema(), (*rule_it).prop_idx, 0 ) )
                        {
                            rule_hit = true;
                        }
                    } // end type = 8

                    if((*rule_it).rule_type == 's' && entry_pre_dict_data_ptr){
                        u2 data_len = 0;
                        // hit when this dict appear.
                        const char* sptr = (const char*)entry_data_ptr->GetData(dicts[dict_id]->GetSchema(),
                            idx->GetStringPool(), (*rule_it).prop_idx, &data_len);
                        if(sptr && strncmp(sptr, (*rule_it).v_str.c_str(), data_len) && data_len == (*rule_it).v_str.length())
                        {
                            rule_hit = true;
                        }
                    } // end type = string

                    if(rule_hit) {
                        if((*rule_it).in_dag) {
                            if(dict_dag_hit_ptr < dict_dag_hit_end)
                                dict_dag_hit_ptr += csr::csrUTF8Encode(dict_dag_hit_ptr, (*rule_it).rule_id );
                        }else {
                            if(dict_seg_hit_ptr < dict_seg_hit_end)
                                dict_seg_hit_ptr += csr::csrUTF8Encode(dict_seg_hit_ptr, (*rule_it).rule_id );
                        }
                    }

                } // check each rule.
            } // for each sub dict's entry.
            // check offset -> append per term's hit.
        }
        // update each entry's dag & seg.
        {
            int hit_dag_string_offset = 0;
            int hit_seg_string_offset = 0;
            term_it = term_hit_string_seg.find(i*entry_size); // check term exist in global.
            bool term_found = ( term_it != term_hit_string_seg.end() );
            *dict_dag_hit_ptr = 0;
            *dict_seg_hit_ptr = 0;
            // save to string pool, get offset
            if( dict_dag_hit_ptr != dict_dag_hit ) {
                if(term_found) {
                    // term + dict(props)
                    std::string hit_s = term_hit_string_dag[i*entry_size] + (const char*)dict_dag_hit;
                    hit_dag_string_offset = _rulehits_pool.AllocString(hit_s.c_str(), hit_s.length());
                }else
                    hit_dag_string_offset = _rulehits_pool.AllocString((const char*)dict_dag_hit, dict_dag_hit_ptr - dict_dag_hit);
            }else
            if(term_found){
                hit_dag_string_offset = _rulehits_pool.AllocString(
                            term_hit_string_dag[i*entry_size].c_str(), term_hit_string_dag[i*entry_size].length());
            }

            if( dict_seg_hit_ptr != dict_seg_hit ) {
                if(term_found) {
                    // term + dict(props)
                    std::string hit_s = term_hit_string_seg[i*entry_size] + (const char*)dict_seg_hit;
                    hit_seg_string_offset = _rulehits_pool.AllocString(hit_s.c_str(), hit_s.length());
                }else
                    hit_seg_string_offset = _rulehits_pool.AllocString((const char*)dict_seg_hit, dict_seg_hit_ptr - dict_seg_hit);
            }
            else
            if(term_found){
                hit_seg_string_offset = _rulehits_pool.AllocString(
                            term_hit_string_seg[i*entry_size].c_str(), term_hit_string_seg[i*entry_size].length());
            }
            // set the offset to seg|dag
            if(hit_dag_string_offset > 0)
                entry_data_ptr->SetU4(idx->GetSchema(), dag_idx, hit_dag_string_offset);

            if(hit_seg_string_offset > 0)
                entry_data_ptr->SetU4(idx->GetSchema(), seg_idx, hit_seg_string_offset);
        }
    } // for each entry
    return 0;
}

const char* SegScript::GetDictionaryName(u2 dict_id) const
{
    mm::DictBase * dict = _dict_mgr->GetDictionary(dict_id);
    if(dict)
        return dict->GetDictName().c_str();
    return NULL;
}

const std::string SegScript::GetDictionaryNames(const char* category) const
{
    return _dict_mgr->GetDictionaryNames(category);
}
int SegScript::GetDictionaryID(const char* dict_name) const
{
    return _dict_mgr->GetDictionaryID(dict_name);
}

void SegScript::_KeepAPICode() {
    //reg_at_char_prepare(_script, 0, NULL);
}

} // namespace mm



