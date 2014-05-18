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


#if !defined(_STRINGPOOLMEMORY_H)
#define _STRINGPOOLMEMORY_H

#include <stdlib.h>
#include <string.h>
#include "csr_typedefs.h"
#include "mm_hashmap.h"

#define STRING_POOL_ENTRY_DATA_SIZE     65535u

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
	
	StringPoolMemoryEntry(u1* ptr, u4 size) : _ptr(ptr), _size(size), _used(size), _next(NULL)
	{
		// 用于从磁盘加载
	}

    int push_string(const char* ptr, u2 len) {
        if( _used + len > _size )
			return -1; // this pool is full.
		//FIXME: check the buffer size ?
        u2* ptru2 = (u2*) (_ptr+_used);
        *ptru2 = len;
        _used += sizeof(u2);
        memcpy(_ptr+_used, ptr, len);
		_used += len;
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
	virtual const char* GetString(u4 offset, u2* data_length);
	virtual u4 GetSize() { return _total_size; }  // 返回连续存储需要的大小。

public:
	int Dump(u1* ptr, u4 size);
	int Load(u1* ptr, u4 size);
	int Reset();

protected:
    StringPoolMemoryEntry* _begin;
    StringPoolMemoryEntry* _current;

	unordered_map<u8, u4> _string_entries;	// 系统已知的 string 值，为了省内存，保存的是数据的 hash code， 而不是数据本身。会造成部分冲突，在词典的应用中，足够了。
    u4  _total_size;	// the total size of string data entry, 

protected:
    void MakeNewEntry();

public:
    // function status code
    static int STATUS_OK;
    static int STATUS_STRING_TOO_BIG;
	static int STATUS_INSUFFICIENT_BUFFER;
};

} // namespace mm

#endif  //_STRINGPOOLMEMORY_H
