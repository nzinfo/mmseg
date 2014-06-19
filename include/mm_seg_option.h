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

#include "mm_dict_mgr.h"
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
    std::string _columns;            // 需要从词典中读取的 property 的名称的列表
    std::string _special_dictname;   // 需要加载的专用词典的名称
    bool _enable_oov;                // 是否启用未登录词识别，隐含的含义是启动 CRF
    bool _enable_ner;                // 是否启用 命名实体识别
    bool _enable_pos;                // 是否启用 词性标注
    bool _enable_pinyin;             // 是否给文字增加注音; 此处的拼音与从词库中读取的拼音不同，此处应该处理多音字。实际中，一般只要求读取注音字段。
    bool _enable_fullseg;            // 输出词库中出现的全部词（作为切分结果的标注）
    bool _enable_keep_origin;        // 输出结果中，保留原始输入的字符串形式，用于精确的，区分大小写的匹配。
    bool _enable_stem;               // 启用 对 英文等 字母语言的词干提取 使用 libstemmer
    bool _enable_script;             // 是否启动 script 层，修正切分结果。

    // annote id 有关
    unordered_map<std::string, u2> _annote2id;
    char*           _annote_name_pool;
    char**          _id2annote;     // 编号到 annote 名字的映射
public:
    SegOptions(mm::DictMgr* dict_mgr, const char* annote, const char* special_dictname )
        :_annotes(annote), _special_dictname(special_dictname), _id2annote(NULL)
    {
        _enable_oov = _enable_ner = _enable_pos = _enable_pinyin = false;
        _enable_stem = _enable_fullseg = _enable_keep_origin = false;
        _enable_script = true;

        // 对 annotes 进行二次处理，去掉 不是词典字段的值
        _annote_name_pool = (char*)malloc(_annotes.length()+2);
        //memcpy(_annote_name_pool, annote, _annotes.length());
        _columns = ProcessAnnotes(dict_mgr, _annotes);
    }

    virtual ~SegOptions() {
        free(_annote_name_pool);
        if(_id2annote)
            free(_id2annote);
    }

    u2 GetAnnoteID(const std::string& annote_name) {
        /*
         * 根据名字，得到本次运行该标引的编号; 从 1 开始
         *
         * 返回 0 该名称不存在；
         *
         * annote_name 不只包括词典中的名字， 是全部可用的 annote_type
         *
         */
        unordered_map<std::string, u2>::iterator it;
        it = _annote2id.find(annote_name);
        if ( it != _annote2id.end() )
            return it->second;
        return 0;
    }

    const char* GetAnnoteName(u2 idx) {
        if(_id2annote && idx <= _annote2id.size()) {
            return _id2annote[idx];
        }
        return NULL;
    }

    const std::string& Columns() {
        // 返回需要从词典文件中读取的属性信息。
        return _columns;
    }

    std::string GetSwitch();
    void SetSwitch(std::string& s);

    std::string& GetSpecialDictionaryName() {
        return _special_dictname;
    }

    // quick flag read
    inline const bool EnableOOV() const { return _enable_oov; }
    inline const bool EnableNER() const { return _enable_ner; }
    inline const bool EnablePOS() const { return _enable_pos; }
    inline const bool EnablePinyin() const { return _enable_pinyin; }
    inline const bool EnableFullSegTerms() const { return _enable_fullseg; }
    inline const bool EnableKeepOrigin() const { return _enable_keep_origin; }
    inline const bool EnableScript() const { return _enable_script; }

protected:
    std::string ProcessAnnotes(mm::DictMgr* dict_mgr, std::string &annotes);
};

} // namespace mm

#endif  //_SEGOPTIONS_H
