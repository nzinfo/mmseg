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

#if !defined(_ENTRYDATAPOOL_H)
#define _ENTRYDATAPOOL_H

#define MAX_ENTRYPOOL_SIZE  65535u      // 最多 65535 条？ 如果是压缩，可能比这个多。

namespace mm {

struct EntryData;
class EntryDataPool;

class EntryDataPoolEntry {
    /*
     *  维持一个链表, 用于存储 entry's pool
     *  MaxCount: 65535
     *  Two type,
     *  - Compat: Load From disk, each entry are compat.
     *  - Normal: Only Normal type can alloc NewEntry.
     */
friend class EntryDataPool;

public:
    EntryDataPoolEntry()
        :_size(0), _used(0),_entry_count(0), _data_ptr(NULL), _next(NULL) {}

    EntryDataPoolEntry(u2 entry_size, u4 entry_count)
        :_entry_count(entry_count), _used(0), _next(NULL){
        _size = entry_size * entry_count;
        _data_ptr = (u1*)malloc(_size);
    }
    EntryDataPoolEntry(u1* ptr, u4 len)
        :_data_ptr(ptr), _size(len), _used(len) {}

    // return NULL if no space for new entry.
    EntryData* NewEntry(u4 entry_size);
    EntryData* GetEntry(u4 offset);

protected:
    u4  _size;
    u4  _used;
    u4  _entry_count; // 目前没用到
    u1* _data_ptr;
    EntryDataPoolEntry* _next;
};

class EntryDataPool {
public:
	/*
	 * 因为在初始化的时刻，可以通过 Schema 得到 Entry 不压缩的尺寸。但是不知道是从磁盘加载还是直接内存分配。此刻并不实际初始化内存。
	 * 当 SetData 或 NewEntry 被调用时，再次判断
	 */

	EntryDataPool(u2 entry_size_uncompressed):
		_entry_size_uncompressed(entry_size_uncompressed) {
			_begin = NULL;
			_current = NULL;
			_updatable = true;
            _entry_next_offset = 0;
	}

	virtual ~EntryDataPool();

public:
    EntryData* NewEntry();
    u4         NewEntryOffset();
	EntryData* GetEntry(u4 offset);

	// Compat EntryDataPool , 
	// Update the _entries mapping;
	// return DataPool's real size (compat size)
	int Compat();
    void SetData(u1* ptr, u4 len); // load from disk. after load from disk, user still can create new entry.
	// return a newly created entry, every field is exactly the same with entry.
	// if origin entry is compat, and _updatable == true, return a newly uncompat engry.
	// if _updatable == false, return NULL.
    EntryData* CloneEntry(const EntryData* entry);
	
	// EntryPool Related
    int MakeNewEntryPool(); // create a new EntryDataPoolEntry 追加到当前的列表上.

public:
	int Dump(u1* ptr, u4 size);
	int Load(u1* ptr, u4 size); //此处应该是 const, 实际也不会修改。但是为了能够 复用代码...
	int Reset();
    u4  GetSize() { return _entry_next_offset; }

protected:
	// Record Entry's index and offset in real databuffer's mapping.
    //map<id, offset> _entries;
    bool _updatable;
	EntryDataPoolEntry *_begin;
	EntryDataPoolEntry *_current;
	u2	_entry_size_uncompressed;
    u4  _entry_next_offset;
};

} // namespace mm

#endif  //_ENTRYDATAPOOL_H
