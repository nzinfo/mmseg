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


#if !defined(_SEGPOLICYMMSEG_H)
#define _SEGPOLICYMMSEG_H

#include "mm_segpolicy.h"

namespace mm {

class SegPolicyMMSeg : public SegPolicy {
public:
    SegPolicyMMSeg()
        :SegPolicy() {
        _ucs2_freq_log = new float[65536];
        _cjk_chartag = 0;
    }
    virtual ~SegPolicyMMSeg() {
        if(_ucs2_freq_log)
            delete[] _ucs2_freq_log;
    }

    /*
     * 使用 MMSeg 算法的切分策略
     */
public:
    virtual int Apply(const DictMgr &dict_mgr, SegStatus& status);
    int BuildUSC2CharFreqMap(const DictMgr &dict_mgr);

private:
    float* _ucs2_freq_log;    // 64K
    u2     _cjk_chartag;
};

} // end namespace mm

#endif  //_SEGPOLICYMMSEG_H
