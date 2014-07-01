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

#if !defined(_DICTMATCHRESULT_H)
#define _DICTMATCHRESULT_H
#include <stdlib.h>
#include <string.h>

#include "csr_typedefs.h"

namespace mm {

/* 
 * 仅供参考 定义为一个 u8 的 union
 */
typedef union DictMatchEntry {
    struct {
        u2 _dict_id;
        u2 _len;
        u4 _value;
    }match;
    u8 v;
}DictMatchEntry;

// An object keep match of PrefixMatch ( ExactMatch return the offset, value instead).
// 
class DictMatchResult {
public:
    // give a default buffer & match, only for script usage.
    DictMatchResult()
        :_pos(0), _max_match(1024) {
        _matches = (DictMatchEntry*)malloc(sizeof(DictMatchEntry)*1024);
        _own_matches = true;
    }

    DictMatchResult(u1* ptr, u4 max_match)  // max of match result. ptr's size = sizeof(DictMatchEntry==u8) * max_match
        :_pos(0), _max_match(max_match), _matches((DictMatchEntry*)ptr)
        ,_own_matches(false)
        {}

    virtual ~DictMatchResult()
    {
        if(_own_matches)
            free(_matches);
    }

    void Reset() {
        memset(_matches, 0, _max_match*sizeof(DictMatchEntry));
        _pos = 0;
    }
    inline int Match(mm::DictMatchEntry& entry) {
        // 添加 新的命中， 如果已满，则返回 -1
        if(_pos<_max_match) {
            _matches[_pos].v = entry.v; // basic type op, ignore default copy construct
            _pos ++;
            return _pos;
        }
        return -1;
    }

    inline const mm::DictMatchEntry* GetMatch(u4 idx) const {
        if(idx<_pos) {
            return &(_matches[idx]);
        }
        return NULL;
    }

    u4   Count() const {
        return _pos;
    }

private:
    u4 _pos;
    u4 _max_match;
    DictMatchEntry* _matches;
    bool _own_matches;
};

} //mm namespace

#endif  //_DICTMATCHRESULT_H
