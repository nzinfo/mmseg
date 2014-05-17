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



#include "mm_dict_base.h"

namespace mm {

int DictBase::Load(const char* fname) {
    return 0;
}

int DictBase::Save(const char* fname, u4 rev) {
    return 0;
}

int DictBase::Init(const char* dict_name, const char* schema_define) {
    return 0;
}

void DictBase::Reset() {

}

/*
int DictBase::Insert(string term) {
    return 0;
}
*/

u4 DictBase::EntryCount() {
    return 0;
}

int DictBase::ExactMatch(const char* q, u2 len) {
    return 0;
}

int DictBase::PrefixMatch(const char* q, u2 len, DictMatchResult* rs) {
    return 0;
}

int DictBase::SaveRaw(const char* fname) {
    return 0;
}

int DictBase::LoadRaw(const char* fname) {
    return 0;
}

int DictBase::MakeUpdatable() {
    return 0;
}

bool DictBase::IsUpdatable() {
    return true;
}

IStringPool & DictBase::GetStringPool() {
    return *_string_pool;
}

EntryData* DictBase::GetEntryData(const char* term) {
    return NULL;
}

/*
EntryData& DictBase::GetEntryData(i4 term_offset) {

}
*/

void DictBase::SetDictionaryId(u2 dict_id_of_mgr) {

}

u4 DictBase::GetReversion() {
    return 0;
}

} //mm namespace

/* -- end of file -- */

