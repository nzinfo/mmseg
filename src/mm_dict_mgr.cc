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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include "win32/dirent.h"
#else
#include "dirent.h"
#endif

#include <vector>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <algorithm>
#include <queue>

#include "mm_dict_mgr.h"

namespace mm {

DictMgr::~DictMgr() {

}

int DictMgr::LoadTerm(const char* dict_path) {
    /*
     *  一个额外的工作，加载字符的转换码表
     */

    std::vector<std::string> charmap_dicts;
    std::vector<std::string> term_dicts;
    // if path have more dict. only the first 20 , 24 solt, 0~3 reversed.
    term_dicts.reserve(20);
    int nfiles = GetDictFileNames(dict_path, ".uni", charmap_dicts);
    return 0;
}

int DictMgr::LoadPharse(const char* dict_path) {
    return 0;
}

int DictMgr::LoadSpecial(const char* dict_path) {
    return 0;
}

int DictMgr::GetDictFileNames(const char* dict_path, std::string fext, std::vector<std::string> &files)
{
    files.clear();
    std::string fname; fname.reserve(255);
    DIR *dir;
    struct stat filestat;
    struct dirent *ent;
    if ((dir = opendir (dict_path)) != NULL) {
      /* print all the files and directories within directory */
      while ((ent = readdir (dir)) != NULL) {
          fname = dict_path;
          fname += "/";
          fname +=ent->d_name;
          if (stat( fname.c_str(), &filestat )) continue;
          if (S_ISDIR( filestat.st_mode ))         continue;

          if( std::string::npos != fname.rfind(fext.c_str(), fname.length() - fext.length(), fext.length()) ) {
              files.push_back(fname);
          }
      } // end while
      closedir (dir);
    } else {
      return 0;
    }
    return files.size();
}

DictBase* DictMgr::GetDictionary(const char* dict_name) {
    return NULL;
}

DictBase* DictMgr::GetDictionary(u2 dict_id) {
    return NULL;
}

DictBase* DictMgr::GetDictionarySepcial(const char* dict_name) {
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
