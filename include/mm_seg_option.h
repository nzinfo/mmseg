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

#if !defined(_SEGOPTIONS_H)
#define _SEGOPTIONS_H

#include "mm_dict_term_user.h"

namespace mm {


// columns:
//    where property we are interest in.
// if a annote not in column, will just be ignored.
// 
// Save 
// 
// dictid, prop_id  -> AnnoteTypeID
// AnnoteTypeID -> [ (dictid, prop_id ) , ]
// 
class SegOptions {
	/*
		分词系统的选项
		- 允许出现的标引
		- 激活的特殊词典
        - 用户词典 (移出， option 不应该与某个切分的 session 有关)
		- 是否允许 script
		- 是否启用未登录词识别
		- 是否启用 NER
		//- 使用的切分策略 mm;crf;script
        记录系统发现的未登录词
	 */
protected:
    std::string _annotes;            // 系统中允许出现的标引名称， 该名称为系统各模块约定。 如不遵守约定，可能冲突。 以 ； 分割 如为空，则为全部允许
    std::string _special_dictname;   // 需要加载的专用词典的名称
    bool _enable_oov;                // 是否启用未登录词识别，隐含的含义是启动 CRF
    bool _enable_ner;                // 是否启用 命名实体识别
    bool _enable_pos;                // 是否启用 词性标注
    bool _enable_pinyin;             // 是否给文字增加注音
    bool _enable_script;             // 是否启动 script 层，修正切分结果。
public:
    SegOptions(const char* annote, const char* special_dictname )
        :_annotes(annote), _special_dictname(special_dictname)
    {
        _enable_oov = _enable_ner = _enable_pos = _enable_pinyin = false;
        _enable_script = true;
    }

    std::string GetSwitch();
    void SetSwitch(std::string& s);

    std::string& GetSpecialDictionaryName() {
        return _special_dictname;
    }
};

} // namespace mm

#endif  //_SEGOPTIONS_H
