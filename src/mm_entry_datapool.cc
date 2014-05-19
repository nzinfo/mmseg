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
 */


#include "mm_entrydata.h"
#include "mm_entry_datapool.h"

namespace mm {

int EntryDataPool::STATUS_OK = 0;
int EntryDataPool::STATUS_INSUFFICIENT_BUFFER = -413;

EntryData* EntryDataPoolEntry::NewEntry(u4 entry_size)
{
    //u1* ptr = (_data_ptr+_used);
    EntryData* entry_ptr = (mm::EntryData*) &_data_ptr[_used];
    _used += entry_size;

    // mark as uncompresed.
    entry_ptr->SetAsUnCompressed();
    return entry_ptr;
}

EntryData* EntryDataPoolEntry::GetEntry(u4 offset)
{
    // FIXME: check the offset 's range?
    if(offset < _size) {
        EntryData* entry_ptr = (mm::EntryData*) &_data_ptr[offset];
        return entry_ptr;
    }
    return NULL;
}

EntryDataPool::~EntryDataPool() {
	Reset();
}

int EntryDataPool::Dump(u1* ptr, u4 size)
{
    /*
     *  此处可以通过写入文件句柄, 但是为了实现简化起见，处理为写入内存。
     */
    if (size < GetSize() )
        return STATUS_INSUFFICIENT_BUFFER;

    EntryDataPoolEntry* pool_ptr = _begin;
    u1* current_ptr = ptr;
    while(pool_ptr) {
        memcpy(current_ptr, pool_ptr->_data_ptr, pool_ptr->_used);
        current_ptr += pool_ptr->_used;
        pool_ptr = pool_ptr->_next;
    }
    return STATUS_OK;
}

int EntryDataPool::Load(u1* ptr, u4 size)
{
    Reset();
    if(_begin == NULL) {
        _begin = new EntryDataPoolEntry(ptr, size);
        _current = _begin;
    }
    return STATUS_OK;
}

int EntryDataPool::Reset()
{

    EntryDataPoolEntry * ptr = _begin;
    EntryDataPoolEntry * prev = NULL;
    while(ptr) {
        prev = ptr;
        ptr = ptr->_next;
        delete prev;
    }
    _begin = _current = NULL;
    _updatable = true;
    _entry_next_offset = 0;
	return 0;
}

EntryData* EntryDataPool::NewEntry() {
	/*
     * Alloca a block of memory, return.
	 */
    if(_current == NULL) {
        // uninit pool.
        EntryDataPoolEntry* entry = new EntryDataPoolEntry(_entry_size_uncompressed, MAX_ENTRYPOOL_SIZE);
        _begin = _current = entry;
    }
    EntryData* entry_ptr = _current->NewEntry(_entry_size_uncompressed);
    if(entry_ptr == NULL) {
        // current pool is full.
        MakeNewEntryPool();
		entry_ptr = _current->NewEntry(_entry_size_uncompressed);
    }

    _entry_next_offset += _entry_size_uncompressed;
    return entry_ptr;
}

u4 EntryDataPool::NewEntryOffset() {
   EntryData* ptr =  NewEntry();
   if(ptr) {
        return _entry_next_offset - _entry_size_uncompressed;
   }
   assert(false); // should never happen.
   return 0;
}

EntryData* EntryDataPool::GetEntry(u4 offset) {
    EntryDataPoolEntry* pool_ptr = _begin;
    while(pool_ptr) {
        if(offset > pool_ptr->_used ) {
            offset -= pool_ptr->_used;
        } else {
            // meet the actually pool
            return pool_ptr->GetEntry(offset);
        }
        pool_ptr = pool_ptr->_next;
    }
    return NULL;
}

int EntryDataPool::Compat() {
    return 0;
}

void EntryDataPool::SetData(u1* ptr, u4 len) {
    Reset();

    EntryDataPoolEntry* entry = new EntryDataPoolEntry(ptr, len);
    _begin = _current = entry;
}

EntryData* EntryDataPool::CloneEntry(const EntryData* entry) {
    return NULL;
}

int EntryDataPool::MakeNewEntryPool() {
    EntryDataPoolEntry* entry = new EntryDataPoolEntry(_entry_size_uncompressed, MAX_ENTRYPOOL_SIZE);

    _current->_next = entry;
    _current = entry;
    return 0;
}

} // mm namespace

/* -- end of file -- */
