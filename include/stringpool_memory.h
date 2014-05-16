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


#if !defined(_STRINGPOOLMEMORY_H)
#define _STRINGPOOLMEMORY_H

#include <stdlib.h>
#include <string.h>
#include "csr_typedefs.h"

#define STRING_POOL_ENTRY_DATA_SIZE     65536

namespace mm {

typedef struct StringPoolMemoryEntry
{
    u1* _ptr;   // memory data range.
	u4 _size;
	u4 _used;
	StringPoolMemoryEntry* _next;
    StringPoolMemoryEntry() : _ptr(NULL), _size(0), _used(0), _next(NULL)
    {
        _ptr = (u1*)malloc(STRING_POOL_ENTRY_DATA_SIZE); // alloc 64K per string pool entry.
        _size = STRING_POOL_ENTRY_DATA_SIZE;
        memset(_ptr, 0, STRING_POOL_ENTRY_DATA_SIZE);
    }
    int push_string(const char* ptr, u2 len) {
        //FIXME: check the buffer size ?
        u2* ptru2 = (u2*) (_ptr+_used);
        *ptru2 = len;
        _used += sizeof(u2);
        memcpy(_ptr+_used, ptr, len);
        return 0;
    }

}StringPoolMemoryEntry;

// A string pool is a linked list of 64K bytes.
//   { size, used, next, ptr_begin, ptr }
// 
class StringPoolMemory {
public:
	StringPoolMemory();
	virtual ~StringPoolMemory();

public:
    virtual i4 AllocString(const char* buf, u2 length);
	// return the string's ptr by string 's offset
	virtual const char* GetString(u4 offset, u4* data_length);
	virtual u4 GetSize();

public:
	int Dump(u1* ptr, u4 size);

protected:
    StringPoolMemoryEntry* _begin;
    StringPoolMemoryEntry* _current;

    u4  _total_size;

    void MakeNewEntry() {
        _current->_next = new StringPoolMemoryEntry();
        _current = _current->_next;
    }

public:
    // function status code
    static int STATUS_OK;
    static int STATUS_STRING_TOO_BIG;
};

} // namespace mm

#endif  //_STRINGPOOLMEMORY_H
