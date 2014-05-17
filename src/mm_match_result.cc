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

#include "mm_match_result.h"
namespace mm {

DictMatchResult::DictMatchResult(u2 max_match) {

}

DictMatchResult::~DictMatchResult() {

}

void DictMatchResult::Reset() {

}

u2 DictMatchResult::Match(u2 dict_id, u2 len, u4 value) {
    return 0;
}

int DictMatchResult::SetData(u1* ptr, u4 len) {
    return 0;
}

} //mm namespace

/* -- end of file -- */
