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



#include "mm_entrydata.h"
#include "mm_dict_schema.h"
#include "iface_stringpool.h"

namespace mm {

// 因为 EntryData* 实际为 u1*，因此不再需要 Load & Dump & Assign.
/*
int  EntryData::Assign(const DictSchema* Schema, const u1* ptr, u2 size)
{
    return 0;
}

int EntryData::Load(const u1* ptr, u2 size) {
    return 0;
}

u2 EntryData::Save(u1* ptr, u2 size) {
    return 0;
}
*/

u4 EntryData::GetCompatSize() {
    return 0;
}

u4 EntryData::GetSize() {
    return 0;
}

int EntryData::Dump(u1* ptr, u2 size, u2 fieldmask, IStringPool* pool) {
    return 0;
}

} // mm namespace

/* -- end of file -- */
