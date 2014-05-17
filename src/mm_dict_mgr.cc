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


#include "mm_dict_mgr.h"

namespace mm {

int DictMgr::Load(const char* dict_path) {
    return 0;
}

DictBase* DictMgr::GetDictionary(const char* dict_name) {
    return NULL;
}

DictBase* DictMgr::GetDictionary(u2 dict_id) {
    return NULL;
}

u2 DictMgr::GetDictionaryIdx(const char* dict_name) {
    return 0;
}

int DictMgr::LoadIndexCache(const char* fname) {
    return 0;
}

int DictMgr::SaveIndexCache(const char* fname) {
    return 0;
}

int DictMgr::BuildIndex() {
    return 0;
}

int DictMgr::Reload() {
    return 0;
}

void DictMgr::UnloadAll() {

}

int DictMgr::VerifyIndex() {
    return 0;
}

int DictMgr::ExactMatch(const char* q, u2 len, DictMatchResult* rs) {
    return 0;
}

int DictMgr::PrefixMatch(const char* q, u2 len, DictMatchResult* rs) {
    return 0;
}

} //mm namespace

/* -- end of file -- */
