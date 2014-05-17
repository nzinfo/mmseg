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

#if !defined(_ENTRYDATA_H)
#define _ENTRYDATA_H

#include "csr_typedefs.h"
#include "mm_dict_schema.h"
#include "iface_stringpool.h"

namespace mm {

// Store Term's Property Data.
// -------------------------------------------
// @ Building
// _is_compate == false, update fields by offset.
// 
// @ Reading
// _is_compate == true, needs calc offset.
// And all SetXX will return ERR_READONLY_ENTRY.

class EntryData {
    /*
     * EntryData 存在2种状态
     * -  实际数据，未压缩格式
     * -  实际数据，压缩格式。
     */
public:
    EntryData():_data_ptr(NULL){}   // if string pool is NULL?
    int Assign(const DictSchema* Schema, const u1* ptr, u2 size); // 分配部分内存给 Entry, 应该使用需要的部分. 返回, 实际用掉的值。一般为Schema的 GetEntryDataSize
    int Load(const u1* ptr, u2 size); // size: the data size after ptr. load data from ptr, [mask, data]
    u2  Save(u1* ptr, u2 size);        // dump to buffer.

    // pass Schema each time, so EntryData can remove a pointer to Schema, and reduce 8byte each entry.
    int SetU2(const DictSchema* Schema, const char* prop, u2 v);
    int SetU4(const DictSchema* Schema, const char* prop, u4 v);
    int SetU8(const DictSchema* Schema, const char* prop, u8 v);
	// SetTheStringValue. In fact store the string offset of the string pool.
	// As a shortcut of SetU4
    int SetData(const DictSchema* Schema, const char* prop, const u1* v, u2 v_size);
    // set prop by index.
    int SetU2(u2 idx, u2 v);
    int SetU4(u2 idx, u4 v);
    int SetU8(u2 idx, u8 v);
    int SetData(u2 idx, IStringPool* pool, const u1* v, u2 v_size);

	u2 GetU2(u2 idx);
	u4 GetU4(u2 idx);
	u8 GetU8(u2 idx);
    const u1* GetData(u2 idx, IStringPool* pool, u2* v_size); // the pool must be extactly the same whom pass to SetData

    u4 GetCompatSize();
    u4 GetSize();
    int Dump(u1* ptr, u2 size, u2 fieldmask, IStringPool* pool);     // 根据字符掩码，将数据Dump到指定的内存区域， 返回实际写入的值, 包括字符串的？


protected:
    //u2      _mask;       // if in memory the highest bit of _mask is 1; else (on disk) it's 0
    u1*       _data_ptr;     // the first 2byte is the mask.
    //u1      _is_compat;
};

} //namespace mm

#endif  //_ENTRYDATA_H
