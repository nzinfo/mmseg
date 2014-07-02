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

#if !defined(_SEGMENTOR_H)
#define _SEGMENTOR_H

#include "mm_dict_mgr.h"
#include "mm_seg_script.h"
#include "mm_seg_status.h"
#include "mm_segpolicy.h"
#include "mm_segpolicy_mmseg.h"

namespace mm {

class SegmentorResultReader {

public:
    virtual int Feed(mm::SegStatus* status) = 0;

    // 返回实际有效的最远的位置， 或 最大的实际字符位置。
    inline u4 icode_last_pos(SegStatus* status) {
        if(!status->_icode_last_s_pos)
            return status->_icode_pos;
        return status->_icode_last_s_pos;
    }

    inline UnicodeSegChar* get_seg_char(SegStatus* status) {
        return status->ActiveBlock()->_icodes;
    }

    inline u4*               get_icodes(SegStatus* status) {
        return status->ActiveBlock()->_icode_chars;
    }

protected:
    // helper function for access SegStatus' protected memeber.
};

class SegmentorResultReaderFile :public SegmentorResultReader
{
public:
    SegmentorResultReaderFile(FILE * fd){
        _fd  = fd;
    }

    // 输出到 int fd, 当然，包括标准输出
    virtual int Feed(SegStatus* status);
protected:
    FILE* _fd;
};

class SegmentorResultReaderScript :public SegmentorResultReader
{
    /*
     *  用于处理脚本 与 结果集 的交互。
     */
public:
    SegmentorResultReaderScript(){
        _icodes = NULL;
        _icode_chars = NULL;
        _icode_lastpos = 0;
    }
    virtual ~SegmentorResultReaderScript() {}

    // 输出到 int fd, 当然，包括标准输出
    virtual int Feed(SegStatus* status);

    // 读取字的标注，用于分词 | 读取 Annote 的数量 （ 目前永远为 0 ）, 如果返回 0 表示 pos 越界
    const char CharAt(int pos, u2* annote_count_at_pos);
    const u4 iCodeLength() {
        return _icode_lastpos;
    }

    // FIXME: add functions for annote
protected:
    UnicodeSegChar* _icodes;
    u4*           _icode_chars;
    u4         _icode_lastpos;
};

class SegmentorResultReaderMM : public SegmentorResultReader
{
    // 输出到文件 ，用 Csft 的 MM 格式， 包括那些属性，由分词服务进程的配置给出。
};

class Segmentor {
public:
    Segmentor(const mm::DictMgr& dict_mgr, const mm::SegScript& script_mgr, const mm::DictTermUser* dict_user=NULL);

public:
    // text_to_seg: the text to proceed.
    // user_dict, special the sass user's custom dictionary, the aspet dict.
    // session_dict/ctx_dict, the dict related to this task
    int Tokenizer(u8 task_id, const char* text_to_seg, u4 text_len, mm::SegStatus* status);

    int GetResult(mm::SegmentorResultReader* r, mm::SegStatus* status) {
        return r->Feed(status);
    }

protected:
    const DictMgr& _dict_mgr;
    const SegScript& _script_mgr;
    const DictTermUser* _dict_user;

    SegPolicyMMSeg _mmseg;  // 各自不同的切分策略，每种策略执行，都把自己的结果设置为 TagB？ 策略待定
    // 脚本对分词结果的处理，也是一种切分策略。
};

} // namespace mm

#endif  //_SEGMENTOR_H
