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

EntryDataPool::~EntryDataPool() {

}

EntryData* EntryDataPool::NewEntry() {
    return NULL;
}

int EntryDataPool::Compat() {
    return 0;
}

void EntryDataPool::SetData(u1* ptr, u4 len) {

}

EntryData* EntryDataPool::CloneEntry(const EntryData* entry) {
    return NULL;
}

} // mm namespace

/* -- end of file -- */
