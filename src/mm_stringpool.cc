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

#include <glog/logging.h>
#include "utils/cityhash.h"
#include "mm_stringpool.h"

namespace mm {

int StringPoolMemory::STATUS_OK = 0;
int StringPoolMemory::STATUS_STRING_TOO_BIG = -413;
int StringPoolMemory::STATUS_INSUFFICIENT_BUFFER = -413;

StringPoolMemory::StringPoolMemory():_current(NULL){
    /*
     * 简化的 String Pool , 可修改的内存形态.
     */
	_begin = NULL;
    //_begin = new StringPoolMemoryEntry();
    //_current = _begin;
    _total_size = 0;
}

StringPoolMemory::~StringPoolMemory(){
    Reset();
}

i4 StringPoolMemory::AllocString(const char* buf, u2 length){
    /*
	 *	0 检查现有的 String 是否已经存在，如果存在，则直接返回。
     *  1 如果当前活跃的 pool 有空闲, 则分配之;
     *  2 如没有空闲, 则分配新的 entry, 长度不能大于 65533,
     *
     *  假定是用于词典等大量短句的应用. return the offset of string in a continue string pool.
	 *
	 *  cityhash with seed ?   0xc3a5c85c97cb3130 , current used default cityhash
	 *    Ref： https://github.com/nevostruev/String-CityHash 
     */

    if(length >= STRING_POOL_ENTRY_DATA_SIZE - 2)
        return STATUS_STRING_TOO_BIG;
	
	// init on needs.
	if(_begin == NULL) {
		_begin = new StringPoolMemoryEntry();
		_current = _begin;
	}

	// check is existed yet.
	u8 hash_id = CityHash64(buf, length);
	{
		unordered_map<u8, u4>::iterator it = _string_entries.find(hash_id);
		if(it != _string_entries.end()) {
			// have exist
			return it->second;
		}
	}

	// check current pool have enough space..  [size(u2), data]
	// FIX: warning C4018
    if( _current->_size < ( _current->_used + length + 2) ) {
        // create new entry.
        MakeNewEntry(); //will update _current
    }
    
	i4 ret_offset = _total_size;
    {
        // make new string.
        _current->push_string(buf, length);
        _total_size += sizeof(u2)+length;
    }
	// regist to map
	{
		_string_entries[hash_id] = ret_offset;
	}
    return ret_offset;
}

void StringPoolMemory::MakeNewEntry() {
	_current->_next = new StringPoolMemoryEntry();
	_current = _current->_next;
	LOG(INFO) << "new string pool entry created.";
}

// return the string's ptr by string 's offset
const char* StringPoolMemory::GetString(u4 offset, u2* data_length){
    /*
     *  offset 处指示的格式
     *  数据长度[2b] 实际数据
     */
    StringPoolMemoryEntry* pool_ptr = _begin;
    while(pool_ptr) {
        if(offset >= pool_ptr->_used ) { //_used 指向的是当前 pool 的最高位未使用的空间。如果 offset 已经指向此处，则当前 pool 必然无数据。
            offset -= pool_ptr->_used;
        } else {
            // meet the actually pool
            *data_length = *( (u2*)(pool_ptr->_ptr + offset) );
            return (char*)pool_ptr->_ptr + offset + sizeof(u2);	// skip the length.
        }
        pool_ptr = pool_ptr->_next;
    }
	*data_length = 0; // no such item.
    return NULL;
}

int StringPoolMemory::Dump(u1* ptr, u4 size) {
	if (size < _total_size)
		return STATUS_INSUFFICIENT_BUFFER;

	StringPoolMemoryEntry* pool_ptr = _begin;
	u1* current_ptr = ptr;
	while(pool_ptr) {
		memcpy(current_ptr, pool_ptr->_ptr, pool_ptr->_used);
		current_ptr += pool_ptr->_used;
		pool_ptr = pool_ptr->_next;
	}
    return STATUS_OK;
}

int StringPoolMemory::Load(u1* ptr, u4 size)
{
	Reset();

	// always true
	if(_begin == NULL) {
		_begin = new StringPoolMemoryEntry(ptr, size);
		_current = _begin;
		_total_size = size;
	}
	return STATUS_OK;
}

int StringPoolMemory::Reset() {
	StringPoolMemoryEntry* entry_ptr = _begin;
	if(entry_ptr) {
		do {
			StringPoolMemoryEntry* prev_ptr = entry_ptr;
			entry_ptr = entry_ptr->_next;
			delete prev_ptr;
		}while(entry_ptr);
	} // end if
	
	_string_entries.clear();

	_begin = NULL;
	_current = NULL;
	_total_size = 0;

	return STATUS_OK;
}

} // namespace mm
