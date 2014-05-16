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
 * THIS SOFTWARE IS PROVIDED BY CYBOZU LABS, INC. ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL CYBOZU LABS, INC. OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Cybozu Labs, Inc.
 *
 */


#if !defined(_ENTRYDATA_H)
#define _ENTRYDATA_H

#include "DictSchema.h"
#include "IStringPool.h"

// Store Term's Property Data.
// -------------------------------------------
// @ Building
// _is_compate == false, update fields by offset.
// 
// @ Reading
// _is_compate == true, needs calc offset.
// And all SetXX will return ERR_READONLY_ENTRY.
class EntryData {
public:
	EntryData(DictSchema Schema, IStringPool pool);
	int Load(const u1* ptr, u2 size);
	u2 Save(u1* ptr, u2 size);
	int SetU2(string prop, u2 v);
	int SetU4(string prop, u4 v);
	int SetU8(string prop, u8 v);
	// SetTheStringValue. In fact store the string offset of the string pool.
	// As a shortcut of SetU4
	int SetString(string prop, string v);
	u4 GetCompatSize();
	u2 GetU2(u2 idx);
	u4 GetU4(u2 idx);
	u8 GetU8(u2 idx);
	const char* GetString(u2 idx);
	u4 GetSize();
	int Dump(u1* ptr, u2 size, u2 fieldmask, IStringPool pool);
protected:
	u2 _mask;
	u1* _data_ptr;
	u1 _is_compat;
};

#endif  //_ENTRYDATA_H
