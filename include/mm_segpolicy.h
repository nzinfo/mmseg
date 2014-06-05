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

#include "mm_seg_status.h"
namespace mm {

class SegPolicy {
public:
	// do char-tag based segmentation, 
	// return how many char been taged, from the begin of SegStatus
    int Apply(SegStatus status) {
        return 0; // tar each char.
    }

    //int Apply(SegStatus status, u1 rs_tag_ptr, 4 rs_len);
    inline u2 GetCharWindowSize() {
        return 0;
    }

    inline u2 GetTermWindowSize() {
        return 0;
    }
};

} // end namespace mm

#endif  //_SEGPOLICY_H
