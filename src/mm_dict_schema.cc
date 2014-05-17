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
#include "mm_dict_schema.h"

namespace mm {

int DictSchema::InitString(const char* schema_define) {
    return 0;
}

std::string DictSchema::GetColumnDefine() {
    return "";
}

const DictSchemaColumn& DictSchema::GetColumn(u2 idx) {
    return _columns[0];
}

void DictSchema::SetDefault(const EntryData& entry_default) {

}

const DictSchemaColumn& DictSchema::GetColumn(const char* column_name) {
    return _columns[0];
}

u4 DictSchema::GetSize() {
    return 0;
}

u2 DictSchema::GetEntryDataSize() {
    return 0;
}

int DictSchema::GetFieldMask(string columns, u2* mask) {
    return 0;
}

} // namespace mm

/* -- end of file -- */
