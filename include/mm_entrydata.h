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
#define GET_ENTRY_DATA(x) ( (u1*) &(x->_data) )

typedef struct EntryData {
    /*
     * EntryData 存在2种状态
     * -  实际数据，未压缩格式
     * -  实际数据，压缩格式。
     * 使用结构体， 而不是类定义 EntryData。 如此，则其可以使用简单内存布局。该对象可以直接从 u1* cast 过来。
     * 警告： 不要再这个结构体中增加新的成员变量。
     */
public:
    //EntryData():_data_ptr(NULL){}   // if string pool is NULL?
    //int Assign(const DictSchema* Schema, const u1* ptr, u2 size); // 分配部分内存给 Entry, 应该使用需要的部分. 返回, 实际用掉的值。一般为Schema的 GetEntryDataSize
    //int Load(const u1* ptr, u2 size); // size: the data size after ptr. load data from ptr, [mask, data]
    //u2  Save(u1* ptr, u2 size);        // dump to buffer.
    inline void SetAsUnCompressed() {
        // this function should be call @ very beginning.
        u1* ptr = (u1*)this;
        // set the mask
        u2* mask_ptr = (u2*)ptr;
        *mask_ptr |= 1<<15; // flag stands for uncompressed data.
    }

    inline bool IsUnCompressed() {
         u2* mask_ptr = (u2*)this;
         // this trick disable warning C4800
         return !! ( ( *mask_ptr ) & ( 1 << 15 ) );
    }

    inline bool HaveColumn(u2 idx) {
         u2* mask_ptr = (u2*)this;
         // this trick disable warning C4800
         return !! ( ( *mask_ptr ) & ( 1 << idx ) );
    }

    // pass Schema each time, so EntryData can remove a pointer to Schema, and reduce 8byte each entry.
    inline int SetU2(const DictSchema* schema, const char* prop, u2 v) {
        const DictSchemaColumn* column = schema->GetColumn(prop);
        if(!column) return -1;
        return SetU2(schema, column->GetIndex(), v);
    }

    inline int SetU4(const DictSchema* schema, const char* prop, u4 v) {
        const DictSchemaColumn* column = schema->GetColumn(prop);
        if(!column) return -1;
        return SetU4(schema, column->GetIndex(), v);
    }

    inline int SetU8(const DictSchema* schema, const char* prop, u8 v) {
        const DictSchemaColumn* column = schema->GetColumn(prop);
        if(!column) return -1;
        return SetU8(schema, column->GetIndex(), v);
    }

	// SetTheStringValue. In fact store the string offset of the string pool.
	// As a shortcut of SetU4
    inline int SetData(const DictSchema* schema, IStringPool* pool, const char* prop, const u1* v, u2 v_size) {
        const DictSchemaColumn* column = schema->GetColumn(prop);
        if(!column) return -1;
        return SetData(schema, column->GetIndex(), pool, v, v_size);
    }

    // set prop by index.
    inline int SetU2(const DictSchema* schema, u2 idx, u2 v){
        if(!IsUnCompressed()) return -1;
        u2 offset = schema->GetColumn(idx).GetOffset();
        u1* ptr = (u1*)this;
        ptr += offset; // the current data pointer.
        *(u2*)ptr = v;
        // update the mask
        u2* mask_ptr = (u2*)this;
        *mask_ptr |= ( 1 << schema->GetColumn(idx).GetIndex() );

        return 0;
    }

    inline int SetU4(const DictSchema* schema, u2 idx, u4 v) {
        if(!IsUnCompressed()) return -1;
        u2 offset = schema->GetColumn(idx).GetOffset();
        u1* ptr = (u1*)this;
        ptr += offset; // the current data pointer.

        *(u4*)ptr = v;
        // update the mask
        u2* mask_ptr = (u2*)this;
        *mask_ptr |= ( 1 << schema->GetColumn(idx).GetIndex() );

        return 0;
    }

    inline int SetU8(const DictSchema* schema, u2 idx, u8 v){
        if(!IsUnCompressed()) return -1;
        u2 offset = schema->GetColumn(idx).GetOffset();
        u1* ptr = (u1*)this;
        ptr += offset; // the current data pointer.

        *(u8*)ptr = v;

        // update the mask
        u2* mask_ptr = (u2*)this;
        *mask_ptr |= ( 1 << schema->GetColumn(idx).GetIndex() );

        return 0;
    }

    inline int SetData(const DictSchema* schema, u2 idx, IStringPool* pool, const u1* v, u2 v_size){
        if(!IsUnCompressed()) return -1;

        u4 v_offset = pool->AllocString((char*)v, v_size);
        return SetU4(schema, idx, v_offset);
    }

    inline u1* GetDataPtr(const DictSchema* schema, u2 idx) {
        // check column exist.
        if(!HaveColumn(idx))
            return NULL;

        u2 mask = * ( (u2*)this );

        u2 offset = schema->GetColumn(idx).GetOffset();
        if(!IsUnCompressed())
           offset = schema->GetCompressedOffset(idx, mask);

        if(offset == 0) {
            // FIXME: this should be an error ?
            return NULL;
        }
        u1* ptr = (u1*)this;
        ptr += offset;
        return ptr;
    }

    u2 GetU2(const DictSchema* schema, u2 idx, u2 default_v = 0) {
        u1* ptr = GetDataPtr(schema, idx);
        if(ptr == NULL)
            return default_v;
        return *(u2*)ptr;
    }

    u4 GetU4(const DictSchema* schema, u2 idx, u4 default_v = 0) {
        u1* ptr = GetDataPtr(schema, idx);
        if(ptr == NULL)
            return default_v;
        return *(u4*)ptr;
    }

    u8 GetU8(const DictSchema* schema, u2 idx, u8 default_v = 0) {
        u1* ptr = GetDataPtr(schema, idx);
        if(ptr == NULL)
            return default_v;
        return *(u8*)ptr;
    }

    // the pool must be extactly the same whom pass to SetData
    const u1* GetData(const DictSchema* schema, u2 idx, IStringPool* pool, u4* v_size) {
        u1* ptr = GetDataPtr(schema, idx);
        if(ptr == NULL)
            return NULL;
        u4 offset = *(u4*)ptr;
        return (const u1*)pool->GetString(offset, v_size);
    }

    u4 GetCompatSize();
    u4 GetSize();
    int Dump(u1* ptr, u2 size, u2 fieldmask, IStringPool* pool);     // 根据字符掩码，将数据Dump到指定的内存区域， 返回实际写入的值, 包括字符串的？

public:
    //u2      _mask;       // if in memory the highest bit of _mask is 1; else (on disk) it's 0
    //u1*       _data_ptr;     // the first 2byte is the mask.
    u1        _data;  // a ungly trick. I want this class act as a pointer to data_pool, EntryData* == u1*
    //u1      _is_compat;
}EntryData;

} //namespace mm

#endif  //_ENTRYDATA_H
