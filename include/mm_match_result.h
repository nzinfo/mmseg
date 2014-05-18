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

#include "csr_typedefs.h"

namespace mm {

/* 
 * 这个类定义，仅供参考， 实际定义为一个 u8 的 union
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
    DictMatchResult(u2 max_match);  // max of match result.
	~DictMatchResult();
	void Reset();
	u2 Match(DictMatchEntry entry);
    int SetData(u1* ptr, u4 len);   // do NOT alloc _matches, reuse the ptr.
private:
    u2 _max_match;
	DictMatchEntry* _matches;
};

} //mm namespace

#endif  //_DICTMATCHRESULT_H
