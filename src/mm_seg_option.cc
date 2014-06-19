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

#include "mm_seg_option.h"
#include "utils/pystring.h"

namespace mm {

std::string SegOptions::GetSwitch() {
    // 使用字符串表示的开关信息
    std::ostringstream oss;
    if(_enable_oov) oss<<"oov;";
    if(_enable_ner) oss<<"ner;";
    if(_enable_pos) oss<<"pos;";
    if(_enable_pinyin) oss<<"pinyin;";
    if(_enable_script) oss<<"script;";
    return oss.str();
}

void SegOptions::SetSwitch(std::string& s) {
    // 使用字符串表示的开关信息
    std::vector<std::string> opts;
    pystring::split(s, opts, ";");
    for(std::vector<std::string>::iterator it = opts.begin(); it != opts.end(); ++it) {
        if(*it == ("oov"))
            _enable_oov = true;
        if(*it == ("ner"))
            _enable_ner = true;
        if(*it == ("pos"))
            _enable_pos = true;
        if(*it == ("pinyin"))
            _enable_pinyin = true;
        if(*it == ("script"))
            _enable_script = true;
    }
}

std::string SegOptions::ProcessAnnotes(DictMgr *dict_mgr, std::string & annotes) {
    /*
     * "pinyin;thes;origin;stem;term" 只保留字典中存在的 columns
     *
     * 0 去掉系统保留的switch
     * 1 查询 dict_mgr 得到全部词典
     * 2 查询 每一个 dict 的 fields，检查
     */
    std::vector<std::string> columns;
    std::vector<std::string> dict_columns;
    pystring::split(annotes, columns, ";");
    char* _annote_name_ptr = _annote_name_pool;

    _annote2id.clear();
    u2 annote_id = 1;
    _id2annote = (char**)malloc( (columns.size()+1) * sizeof(char*));

    for(std::vector<std::string>::iterator it = columns.begin(); it < columns.end(); it++ )
    {
        {
            _annote2id[*it] = annote_id; 
            memcpy(_annote_name_ptr, (*it).c_str(), (*it).length());
            _id2annote[annote_id] = _annote_name_ptr;
            _annote_name_ptr += (*it).length();
            *_annote_name_ptr = 0;
            _annote_name_ptr ++;
			annote_id++;
        }

        if(*it== "origin")
        {    _enable_keep_origin = true; continue; }
        if(*it== "stem")
        {    _enable_stem = true; continue; }
        if(*it== "term")
        {    _enable_fullseg = true; continue; }

        // 检查字段是否存在
        if(dict_mgr->FieldExist(*it)) {
            // add to column
            dict_columns.push_back(*it);
        }
    }
    return pystring::join(";", dict_columns);
}

} // namespace mm



