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

#if !defined(_ENTRYDATA_SCRIPT_H)
#define _ENTRYDATA_SCRIPT_H
#include <assert.h>

#include "csr_typedefs.h"
#include "mm_entrydata.h"

namespace mm {

// EntryData 的 Script 接口

class EntryDataWrap {
    /*
     * 用于让脚本访问 EntryData， 比较丑陋
     * 因此，不提供通过ColumnID 访问属性的方式。
     */

public:
    // pass Schema each time, so EntryData can remove a pointer to Schema, and reduce 8byte each entry.
    int SetU2(mm::EntryData* entry, const mm::DictSchema* schema, const char* prop, u2 v) {
        return entry->SetU2(schema, prop, v);
    }

    int SetU4(mm::EntryData* entry, const mm::DictSchema* schema, const char* prop, u4 v) {
        return entry->SetU4(schema, prop, v);
    }

    int SetU8(mm::EntryData* entry, const mm::DictSchema* schema, const char* prop, u8 v) {
        return entry->SetU8(schema, prop, v);
    }

	// SetTheStringValue. In fact store the string offset of the string pool.
	// As a shortcut of SetU4
    int SetData(mm::EntryData* entry, const mm::DictSchema* schema, mm::IStringPool* pool, const char* prop, const u1* v, u2 v_size) {
        return entry->SetData(schema, pool, prop, v, v_size);
    }


    u2 GetU2(mm::EntryData* entry, const mm::DictSchema* schema, const char* prop, u2 default_v = 0) {
        const DictSchemaColumn* column = schema->GetColumn(prop);
        return entry->GetU2(schema, column->GetIndex(), default_v);
    }

    u4 GetU4(mm::EntryData* entry, const mm::DictSchema* schema, const char* prop, u4 default_v = 0) {
        const DictSchemaColumn* column = schema->GetColumn(prop);
        return entry->GetU4(schema, column->GetIndex(), default_v);
    }

    u8 GetU8(mm::EntryData* entry, const mm::DictSchema* schema, const char* prop, u8 default_v = 0) {
        const DictSchemaColumn* column = schema->GetColumn(prop);
        return entry->GetU8(schema, column->GetIndex(), default_v);
    }

    // the pool must be extactly the same whom pass to SetData
    const u1* GetData(mm::EntryData* entry, const mm::DictSchema* schema, mm::IStringPool* pool, const char* prop, u2* v_size) {
        const DictSchemaColumn* column = schema->GetColumn(prop);
        return entry->GetData(schema, pool, column->GetIndex(), v_size);
    }
};

} //namespace mm

#endif  //_ENTRYDATA_SCRIPT_H
