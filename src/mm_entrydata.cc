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

int EntryData::SetU2(const DictSchema* Schema, const char* prop, u2 v)
{
    return 0;
}

int EntryData::SetU4(const DictSchema* Schema, const char* prop, u4 v)
{
    return 0;
}

int EntryData::SetU8(const DictSchema* Schema, const char* prop, u8 v)
{
    return 0;
}

int EntryData::SetData(const DictSchema* Schema, const char* prop, const u1* v, u2 v_size)
{
    return 0;
}

// set prop by index.
int EntryData::SetU2(u2 idx, u2 v)
{
    return 0;
}

int EntryData::SetU4(u2 idx, u4 v)
{
    return 0;
}

int EntryData::SetU8(u2 idx, u8 v)
{
    return 0;
}

int EntryData::SetData(u2 idx, IStringPool* pool, const u1* v, u2 v_size)
{
    return 0;
}

u2 EntryData::GetU2(u2 idx) {
    return 0;
}

u4 EntryData::GetU4(u2 idx) {
    return 0;
}

u8 EntryData::GetU8(u2 idx) {
    return 0;
}

const u1* EntryData::GetData(u2 idx, IStringPool* pool, u2* v_size){
    return NULL;
}

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
