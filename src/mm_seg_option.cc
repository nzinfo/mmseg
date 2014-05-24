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

} // namespace mm



