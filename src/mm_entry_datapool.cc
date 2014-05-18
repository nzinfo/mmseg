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

EntryData* EntryDataPoolEntry::NewEntry(u4 entry_size)
{
    //u1* ptr = (_data_ptr+_used);
    EntryData* entry_ptr = (mm::EntryData*) &_data_ptr[_used];
    _used += entry_size;

    // mark as uncompresed.
    entry_ptr->SetAsUnCompressed();
    return entry_ptr;
}

EntryDataPool::~EntryDataPool() {
	Reset();
}

int EntryDataPool::Dump(u1* ptr, u4 size)
{
	return 0;
}

int EntryDataPool::Load(u1* ptr, u4 size)
{
	return 0;
}

int EntryDataPool::Reset()
{
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
    return entry_ptr;
}

int EntryDataPool::Compat() {
    return 0;
}

void EntryDataPool::SetData(u1* ptr, u4 len) {

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
