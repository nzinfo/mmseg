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

#include <glog/logging.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#else
#include "dirent.h"
#endif

#include <vector>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <algorithm>
#include <queue>

extern "C" {
#if WIN32
#include "win32/dirent.h"
#include "win32/realpath_win32.h"
#endif
} // end extern C
#include "utils/pystring.h"

#include "mm_dict_mgr.h"
#include "mm_dict_updatable.h"


namespace mm {

typedef struct EntryInDict {
    u1 dict_id;
    u4 entry_offset;
    EntryInDict* next;
}EntryInDict;

//typedef std::vector< EntryInDict > EntryInDictList;

DictMgr::~DictMgr() {
   this->UnloadAll();
   SafeDelete(_mapper);
   SafeDelete(_delta_dictionary);
}

int DictMgr::LoadTerm(const char* dict_path) {
    /*
     *  一个额外的工作，加载字符的转换码表
     */

    std::vector<std::string> charmap_dicts;
    // if path have more dict. only the first 20 , 24 solt, 0~3 reversed.
    int nfiles = GetDictFileNames(dict_path, ".uni", charmap_dicts);
	// if more than one file , accept the 1st.
    {
        // _mapper 不需要 Reset | Clear， Load 的同时清除
        _mapper->Load(charmap_dicts[0].c_str());
        _charmap_fname = charmap_dicts[0];
        LOG(INFO) << "load charmap file " << charmap_dicts[0];
    }
	// if none, break.
    CHECK_GT(nfiles, 0) << "none *.uni file found, exit";

    // Load Term Dictionarys.
    _terms_fname.clear();
    nfiles = GetDictFileNames(dict_path, ".term", _terms_fname);
    {
        for(int i=0; i<MAX_TERM_DICTIONARY; i++) {
            if(_term_dictionaries[i] != NULL )
                delete _term_dictionaries[i];
            _term_dictionaries[i] = NULL;
        }

        // _mapper 不需要 Reset | Clear， Load 的同时清除
        for(int i=0; i<min(nfiles, MAX_TERM_DICTIONARY); i++) {
            _term_dictionaries[i] = new mm::DictTerm();
            _term_dictionaries[i]->Load(_terms_fname[i].c_str());
            _terms_fname.push_back(_terms_fname[i]);
		}
    }
    return 0;
}

int DictMgr::LoadPharse(const char* dict_path) {
    // FIXME: 加载短语资料, Pharse 的 需要与主索引的 Key 合并, 扩展名为 phs
    return 0;
}

int DictMgr::LoadSpecial(const char* dict_path) {
    // FIXME: 加载专用词表 扩展名为 .dict ， 实际就是 basic term.
    return 0;
}

int DictMgr::GetDictFileNames(const char* dict_path, std::string fext, std::vector<std::string> &files)
{
    char resolved_dict_buf[255];
    files.clear();
    std::string fname; fname.reserve(255);
    DIR *dir;
    struct stat filestat;
    struct dirent *ent;

    const char* resolved_dict_path = realpath(dict_path, resolved_dict_buf);

    if ((dir = opendir (resolved_dict_path)) != NULL) {
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
    /*
     *  根据名字，返回词典对象， 包括 专用词表
     *  - GetDictionarySepcial 包括在内
     */
	unordered_map<std::string, mm::DictBase*>::iterator it = _name2dict.find(dict_name);
	if(it == _name2dict.end() )
		return NULL;
    return it->second;
}

DictBase* DictMgr::GetDictionary(u2 dict_id) {
    /*
     *  根据 id 返回， 只能是系统的 Term & Pharse
     */
    unordered_map<u2, std::string>::iterator it = _id2name.find(dict_id);
    if(it == _id2name.end() )
        return NULL;
    // got a name
    return GetDictionary(it->second.c_str());
}

int DictMgr::LoadIndexCache(const char* fname) {
    /*
     * 词条， (dictid:u1, offset:u4)
     * 不能每个词条一个属性，因为词库的属性最多 15 个。
     */
    return 0;
}

int DictMgr::SaveIndexCache(const char* fname) {
    return 0;
}

int DictMgr::BuildIndex() {
    _name2dict.clear();
    _id2name.clear();
    // build dict_name, dict  & build dict_id, dict_name
    int total_entry = 0;
    for(int i=0; i<MAX_TERM_DICTIONARY; i++) {
        if(_term_dictionaries[i] != NULL ) {
            const std::string &dict_name = _term_dictionaries[i]->GetDictName();
            _name2dict[dict_name] = _term_dictionaries[i];
            _id2name[DICTIONARY_BASE+i] = dict_name;
            total_entry += _term_dictionaries[i]->EntryCount();
        }
    }
    for(int i=0; i<MAX_PHARSE_DICTIONARY; i++) {
        if(_pharse_dictionaries[i] != NULL ) {
            const std::string &dict_name = _pharse_dictionaries[i]->GetDictName();
            _name2dict[dict_name] = _pharse_dictionaries[i];
            _id2name[DICTIONARY_BASE+MAX_TERM_DICTIONARY+i] = dict_name;
            total_entry += _pharse_dictionaries[i]->EntryCount();
        }
    }
    // load special, no global index in special
    for(std::vector<DictTerm*>::iterator it = _special_dictionary.begin();
        it != _special_dictionary.end(); ++it) {
        if(*it) {
            const std::string &dict_name = (*it)->GetDictName();
            _name2dict[dict_name] = *it;
        }
    }

    // build global darts index.
    {
        // debug code.
		/*
        u2 key_len = 0;
        for(int i=0; i<_term_dictionaries[0]->EntryCount(); i++) {
            const char* ptr = _term_dictionaries[0]->GetDiskEntryByIndex(i, &key_len, NULL);
            printf("%*.*s/o ", key_len, key_len, ptr);
        }
		*/
		/*
		 *	use quick & dirty struct. map<std::string, dict_list>	dict_list = std::vector<dict_hit>	dict_hit = (dict_id, entry_offset)
		 *  encode dict_list -> string , save as string.
         *
         *  Very slow, huge deletes.
		 */

        unordered_map<std::string, EntryInDict*> keys;
        keys.reserve(total_entry);

        u2 key_len = 0;
        u4 entry_offset = 0;
        // alloc all possible entryindict.
        EntryInDict* pool = (EntryInDict*) malloc( total_entry * sizeof(EntryInDict) );
        EntryInDict* pool_ptr = pool;

        EntryInDict entry;

        unordered_map<std::string, EntryInDict*>::iterator it;
        for(int i=0; i<MAX_TERM_DICTIONARY; i++) {
            if(_term_dictionaries[i] != NULL ) {
                entry.dict_id = i + DICTIONARY_BASE;
                for(u4 j=0; j<_term_dictionaries[i]->EntryCount(); j++) {
                    const char* ptr = _term_dictionaries[i]->GetDiskEntryByIndex(j, &key_len, &entry_offset);
                    //printf("%*.*s/o ", key_len, key_len, ptr);
                    std::string key(ptr, key_len);
                    pool_ptr->dict_id = entry.dict_id;
                    pool_ptr->entry_offset = entry_offset;
                    pool_ptr->next = NULL;

                    it = keys.find(key);
                    if(it == keys.end()) {
                        // create new

                        keys[key] = pool_ptr;
                    } else{
                        // fill the nex
                        EntryInDict* key_entry_ptr = it->second;
                        while (key_entry_ptr->next)
                            key_entry_ptr = key_entry_ptr->next;
                        key_entry_ptr->next = pool_ptr;
                    }
                    pool_ptr ++;
                }
            } // end if _term_dictionaries
        } // end for

        for(int i=0; i<MAX_PHARSE_DICTIONARY; i++) {
            if(_pharse_dictionaries[i] != NULL ) {
                entry.dict_id = i + DICTIONARY_BASE + MAX_TERM_DICTIONARY;
                for(u4 j=0; j<_pharse_dictionaries[i]->EntryCount(); j++) {
                    const char* ptr = _pharse_dictionaries[i]->GetDiskEntryByIndex(j, &key_len, &entry_offset);

                    std::string key(ptr, key_len);
                    pool_ptr->dict_id = entry.dict_id;
                    pool_ptr->entry_offset = entry_offset;
                    pool_ptr->next = NULL;

                    it = keys.find(key);
                    if(it == keys.end()) {
                        // create new
                        keys[key] = pool_ptr;
                    } else{
                        // fill the nex
                        EntryInDict* key_entry_ptr = it->second;
                        while (key_entry_ptr->next)
                            key_entry_ptr = key_entry_ptr->next;
                        key_entry_ptr->next = pool_ptr;
                    }
                    pool_ptr ++;
                }
            }
        }
        // all data collected.
        LOG(INFO) << "gather all terms, count is " << keys.size();
        {
            // build the global index.
        }
        // free
        keys.clear();
        free(pool);
    }
    return 0;
}

int DictMgr::Reload() {
    // unload all & reload by name. ， 有可能文件已经被删除。加载前需要检查

    return 0;
}

void DictMgr::UnloadAll() {
	for(int i=0; i<MAX_TERM_DICTIONARY; i++) {
		if(_term_dictionaries[i] != NULL )
			delete _term_dictionaries[i];
		_term_dictionaries[i] = NULL;
	}
    for(int i=0; i<MAX_PHARSE_DICTIONARY; i++) {
        if(_pharse_dictionaries[i] != NULL )
            delete _pharse_dictionaries[i];
        _pharse_dictionaries[i] = NULL;
	}
    if(_delta_dictionary)
        delete _delta_dictionary;
    _delta_dictionary = NULL;

    // clear special
    for(std::vector<DictTerm*>::iterator it = _special_dictionary.begin();
        it != _special_dictionary.end(); ++it) {
        if(*it) {
            delete *it;
        }
        *it = NULL;
    }
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
