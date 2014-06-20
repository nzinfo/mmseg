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

#if !defined(_DICTMGR_H)
#define _DICTMGR_H

#include "mm_charmap.h"
#include "mm_dict_base.h"
#include "mm_dict_pharse.h"
#include "mm_dict_term.h"


// total 32, 4 for reverse; 20 for term 8 for pharse
#define DICTIONARY_BASE     4
#define MAX_TERM_DICTIONARY 20
#define MAX_PHARSE_DICTIONARY 8
#define TOTAL_DICTIONARY_COUNT 32

namespace mm {

typedef struct BaseDictColumnReadMarker{
    u2          dict_id; // 如果是用户自定义词典等，使用预先保留的值
    char        column_datatype;    // 对应的数据类型 u1 u2 u4 string
    u2          prop_dict_idx;  // 对应的在 EntryData 中未压缩的值
    u2          prop_id; // 额外的信息，在Options 中，该属性的 Annote 的编号
}BaseDictColumnReadMarker;

typedef std::vector<BaseDictColumnReadMarker> BaseDictColumnReadMarkerList;

class DictUpdatable;

// The global term index's format.
// 
// key, value (offset)
// 
// data = dictmask (4u), values of the term in other dicts.
// 
// {
//      1 foreach in dicts
//      2 is the dict have such property?
//      3 entry have such property ?
// }
class DictMgr {
    /*
     * 用于进行 词典 & 规则的管理。因为规则也是某种构词法
     *  - 不再额外增加 ScriptMgr， 内部实现上，保留一个 ScriptMgr 的代理类
     *  * 词典分 4 种
     *    （1） 基础词条， 也是最常见的词条，为系统提供词条；
     *    （2） 短语，为系统提供短句、成语？的标注。可以部分的用于消歧
     *    （3） Special， 基于Aspect或SASS租户的词库，用于定义某一行业的词库
     *    （4） User/Content 具体用户实例， 往往被对话、编辑任务的上下文使用。
     *         - 系统缓存用户的词典（假定系统的用户均为善意）
     *         - 用户的上下文词典，采用 JSON 的形式传递
     *         - 用户的词典的词条属性，需要特殊处理。
     *    （-） Updatable 可更新的词典，目前不支持，用于实时的在线更新
     *
     *  对应的词典名称为 special, session, delta
     *  - 需要在词典初始化的时候，检查名称，避免使用系统的名字
     */
public:
    DictMgr() {
        memset(_term_dictionaries, 0, sizeof(_term_dictionaries));
        memset(_pharse_dictionaries, 0, sizeof(_pharse_dictionaries));
        _delta_dictionary = NULL;
        _global_idx = NULL;

        _mapper = new mm::CharMapper(true);
    }
    virtual ~DictMgr();

public:
    // 加载基本词条 , 格式与 DictBase 一样， 根据扩展名区分
    int LoadTerm(const char* dict_path);
    // 加载短语
    int LoadPharse(const char* dict_path);
    // 加载专有词典， 与其他词典不同， Special 不占用 32 个固定的词典槽
    int LoadSpecial(const char* dict_path);

    const mm::CharMapper* GetCharMapper() const {
        return _mapper;
    }

    /*
     * 得到用于检索全局词条的 DictBase* 的指针。
     */
    mm::DictGlobalIndex* GetGlobalTermIndex(const char* dict_name){
        return _global_idx;
    }

    mm::DictBase* GetDictionary(const char* dict_name) const;
    int GetDictionaryID(const char* dict_name) const;
    // 性能并不比 直接用名字好，仅仅是为了检查加载的情况
    mm::DictBase* GetDictionary(u2 dict_id) const; //not for special & user
    //u2 GetDictionaryIdx(const char* dict_name);
    const std::string GetDictionaryNames(const char* category) const; // 返回词典的 dict_name 的列表， category=NULL|term|pharse|special

    // 根据基本词条和短语 构建唯一的 darts 检索表
    int LoadIndexCache(const char* fname);
    int SaveIndexCache(const char* fname);
    int BuildIndex(bool bRebuildGlobalIdx = false);
    int VerifyIndex();

    // 重新加载词典
	int Reload();
	void UnloadAll();

	// As we have many dictionary, each diction might contain same term.
	// Even a single term might case multi hit-entry, so we needs MatchResult here.
    /*
     * 此处返回的并不是实际的 Match 结果，
     * Segmentor 仍然需要根据 User > Sepcial 填充 DictMatchResult
     */
    int ExactMatch(const char* q, u2 len, mm::DictMatchResult* rs);
    int PrefixMatch(const char* q, u2 len, mm::DictMatchResult* rs);

	int ExactMatch(u4* q, u2 len, mm::DictMatchResult* rs);
    int PrefixMatch(u4* q, u2 len, mm::DictMatchResult* rs, bool extend_value=true);

    int GetMatchByDictionary(const mm::DictMatchEntry* entry, u2 term_len, mm::DictMatchResult* rs) const;

    mm::DictUpdatable* GetUpdatableDictionary() {
        return NULL;    // 目前不支持在线的更新
    }

    // 处理与字段有关
    bool FieldExist(const std::string& field_name){
        unordered_map<std::string, BaseDictColumnReadMarkerList>::const_iterator field_it;
        field_it = _fields.find(field_name);
        return ( field_it != _fields.end() );
    }
    const mm::BaseDictColumnReadMarkerList* GetFieldMarkers(const std::string& field_name) const {
        unordered_map<std::string, BaseDictColumnReadMarkerList>::const_iterator field_it;
        field_it = _fields.find(field_name);
        if(field_it == _fields.end())
            return NULL;
        return &(field_it->second);
    }

public:
    static int GetDictFileNames(const char* dict_path, std::string fext, std::vector<std::string> &files);
    static int GetDictFileNames(const char* dict_path, std::string fext, bool filename_only,
                                std::vector<std::string> &files);

protected:
    CharMapper* _mapper;
    DictTerm* _term_dictionaries[MAX_TERM_DICTIONARY];
    DictUpdatable* _delta_dictionary;
    DictPharse* _pharse_dictionaries[MAX_PHARSE_DICTIONARY];
    std::vector<DictTerm*> _special_dictionary;

    mm::DictGlobalIndex* _global_idx;
    int                  _global_idx_entry_propidx; // 保存 entries 的 为位置，因为新增加了两个字段 so...

    unordered_map<std::string, mm::DictBase*> _name2dict;
    unordered_map<u2, std::string> _id2name;

    // for reload
    std::string _charmap_fname;
    std::vector<std::string> _terms_fname;
    std::vector<std::string> _pharses_fname;
    std::vector<std::string> _specials_fname;

    // for annote & script  <字段名， 包括该字段的词典的该字段读取方式的描述的列表>
    unordered_map<std::string, BaseDictColumnReadMarkerList> _fields;
};

} // namespace mm

#endif  //_DICTMGR_H
