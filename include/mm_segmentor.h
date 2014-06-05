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

namespace mm {

class Segmentor {
public:
    const DictMgr& _dict_mgr;
    const SegScript& _script_mgr;
	// text_to_seg: the text to proceed.
	// user_dict, special the sass user's custom dictionary, the aspet dict.
	// session_dict/ctx_dict, the dict related to this task
    int Tokenizer(u8 task_id, const char* text_to_seg, u4 text_len, SegStatus* status);
    Segmentor(const DictMgr& dict_mgr, const SegScript& script_mgr);
};

} // namespace mm

#endif  //_SEGMENTOR_H
