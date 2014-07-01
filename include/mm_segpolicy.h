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

#if !defined(_SEGPOLICY_H)
#define _SEGPOLICY_H

#include "mm_dict_mgr.h"
#include "csr_typedefs.h"

namespace mm {

class SegStatus;

class SegPolicy {
public:
    // do char-tag based segmentation, 
    // return how many char been taged, from the begin of SegStatus
    virtual int Apply(const DictMgr &dict_mgr, SegStatus& status) {
        return 0; // tar each char.
    }

    //int Apply(SegStatus status, u1 rs_tag_ptr, 4 rs_len);
    inline u2 GetCharWindowSize() {
        return 0;
    }

    inline u2 GetTermWindowSize() {
        return 0;
    }

    int BindAnnote(const DictMgr &dict_mgr, SegStatus& status);

protected:
    //BaseDictColumnReadMarkerList _prop2annote;
    BaseDictColumnReadMarkerList _prop2annote_map[TOTAL_DICTIONARY_COUNT]; // use dict_id as the index.
};

} // end namespace mm

#endif  //_SEGPOLICY_H
