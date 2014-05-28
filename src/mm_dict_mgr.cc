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

#include <fstream>

#include "utils/pystring.h"
#include "utils/utf8_to_16.h"
#include "mm_dict_mgr.h"
#include "mm_dict_updatable.h"


namespace mm {

typedef struct EntryInDict {
    u1 dict_id;
    u4 entry_offset;
    EntryInDict* next;
}EntryInDict;

u2 encode_entry_in_dict(EntryInDict *entry, u1* entries){
    u1* ptr = entries;
    while(entry) {
        *ptr = entry->dict_id;
        ptr ++;
        ptr += csr::csrUTF8Encode(ptr, entry->entry_offset);
        entry = entry->next;
    }
    *ptr = 0;
    return (u2)(ptr - entries);
}

u2 decode_entry_to_matchentry(const u1* entries, u2 data_len, u2 term_len, DictMatchResult* rs){
   const u1* ptr = entries;
    u2  step_len = 0;
    DictMatchEntry m;
    int cnt = 0;

    while(*ptr && (ptr < entries + data_len) ) {
        m.match._dict_id = *ptr;
        ptr ++;
        m.match._len = term_len;
        m.match._value = csr::csrUTF8Decode(ptr, step_len);
        rs->Match(m);

        ptr += step_len;
        cnt ++;
    };
    return cnt;
}

struct Hash_Func
{
    //BKDR hash algorithm，有关字符串hash函数，可以去找找资料看看
    int operator()(char * str)const
    {
        int seed = 131;//31  131 1313 13131131313 etc//
        int hash = 0;
        while(*str)
        {
            hash = (hash * seed) + (*str);
            str ++;
        }

        return hash & (0x7FFFFFFF);
    }
};

struct Cmp
{
    bool operator()(const char *str1,const char * str2)const
    {
        return strcmp(str1,str2) == 0;
    }
};

//typedef unordered_map<const char* , EntryInDict*, Hash_Func, Cmp> keymap;

typedef unordered_map<std::string, EntryInDict* > keymap;

//typedef std::vector< EntryInDict > EntryInDictList;

DictMgr::~DictMgr() {
   this->UnloadAll();
   SafeDelete(_mapper);
   SafeDelete(_delta_dictionary);
   SafeDelete(_global_idx);
}

int DictMgr::LoadTerm(const char* dict_path) {
    /*
     *  一个额外的工作，加载字符的转换码表
     */

    std::vector<std::string> charmap_dicts;
    // if path have more dict. only the first 20 , 24 solt, 0~3 reversed.
    int nfiles = GetDictFileNames(dict_path, ".uni", charmap_dicts);
	// if more than one file , accept the 1st.
	if(nfiles)
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

        for(int i=0; i<min(nfiles, MAX_TERM_DICTIONARY); i++) {
            _term_dictionaries[i] = new mm::DictTerm();
            _term_dictionaries[i]->Load(_terms_fname[i].c_str());
            _terms_fname.push_back(_terms_fname[i]);
            LOG(INFO) << "load term " << _terms_fname[i] << " named " << _term_dictionaries[i]->GetDictName() << " @pos " << DICTIONARY_BASE + i;
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
     *
     * 目前不对 cache 与 词库的更新状态进行检测， 或者， 不使用 cacheindex 功能
     */
    if(!_global_idx) {
        // FIXME: dup code. & and should reset _global_idx @ each load cache.
        _global_idx = new mm::DictBase();
        _global_idx->Init("com.coreseek.mm.global_idx", "entries:s"); // entries all the dict.
    }
    // checkfile.
    std::ifstream dfile(fname, std::ios::binary);
    if(!dfile)
        return -1; //file not found.
    dfile.close();
    // reopen and load.
    return _global_idx->Load(fname);
}

int DictMgr::SaveIndexCache(const char* fname) {
    if(!_global_idx)
        return -1;

    return _global_idx->Save(fname, 1);
}

int DictMgr::BuildIndex(bool bRebuildGlobalIdx) {
    _name2dict.clear();
    _id2name.clear();
    // build dict_name, dict  & build dict_id, dict_name
    int total_entry = 0;
    int total_stringpool_size = 0;

    for(int i=0; i<MAX_TERM_DICTIONARY; i++) {
        if(_term_dictionaries[i] != NULL ) {
            const std::string &dict_name = _term_dictionaries[i]->GetDictName();
            _name2dict[dict_name] = _term_dictionaries[i];
            _id2name[DICTIONARY_BASE+i] = dict_name;
            total_entry += _term_dictionaries[i]->EntryCount();
            u4 string_size = _term_dictionaries[i]->StringPoolSize();
			total_stringpool_size += string_size;
        }
    }
    for(int i=0; i<MAX_PHARSE_DICTIONARY; i++) {
        if(_pharse_dictionaries[i] != NULL ) {
            const std::string &dict_name = _pharse_dictionaries[i]->GetDictName();
            _name2dict[dict_name] = _pharse_dictionaries[i];
            _id2name[DICTIONARY_BASE+MAX_TERM_DICTIONARY+i] = dict_name;
            total_entry += _pharse_dictionaries[i]->EntryCount();
            total_stringpool_size += _term_dictionaries[i]->GetStringPool()->GetSize();
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

	/*
    const char kChineseSampleText[] = {-28, -72, -83, -26, -106, -121, 0};
    {
        //int n = mgr.ExactMatch(kChineseSampleText, 6, &rs);
        //printf("find %d hits\n", n);
        //for(int i=0; i<n; i++)
        {
            //printf("dict_id %d, rs=%d ", rs.GetMatch(i)->match._dict_id, rs.GetMatch(i)->match._value);
            // dump pinyin ,  std mmseg have no pinyin , should output as NULL.
            mm::DictBase* dict = _term_dictionaries[0]; //mgr.GetDictionary( rs.GetMatch(i)->match._dict_id );
            i4 query_entry_offset = dict->ExactMatch( kChineseSampleText, 6 );

            mm::EntryData* entry = dict->GetEntryDataByOffset( query_entry_offset );
            std::string s = dict->GetSchema()->GetColumnDefine();
            printf("schema = %s name= %s offset = %d \n", s.c_str(), dict->GetDictName().c_str(), query_entry_offset);
            const mm::DictSchemaColumn* column = dict->GetSchema()->GetColumn("pinyin");
            if(column) {
                u2 data_len = 0;
                const char* sptr = (const char*)entry->GetData(dict->GetSchema(), dict->GetStringPool(), column->GetIndex(), &data_len);
                printf("%*.*s/x ", data_len, data_len, sptr);
            }
        }
    }
	*/

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
         *  might cause huge memory consume
		 */

        keymap keys;  // 必须包括字符串的原始信息
        keys.reserve(total_entry);

        u2 key_len = 0;
        u4 entry_offset = 0;
        // alloc all possible entryindict.
        EntryInDict* pool = (EntryInDict*) malloc( total_entry * sizeof(EntryInDict) );
        EntryInDict* pool_ptr = pool;
        //char * string_pool = (char*)malloc(total_stringpool_size + total_entry); //add more space, unness.
        //memset(string_pool, 0, total_stringpool_size + total_entry);

        //char * string_pool_ptr = string_pool;

        EntryInDict entry;

        keymap::iterator it;
        for(int i=0; i<MAX_TERM_DICTIONARY; i++) {
            if(_term_dictionaries[i] != NULL ) {
                entry.dict_id = i + DICTIONARY_BASE;
                for(u4 j=0; j<_term_dictionaries[i]->EntryCount(); j++) {
                    const char* ptr = _term_dictionaries[i]->GetDiskEntryByIndex(j, &key_len, &entry_offset);
					/*
                    {
                        if(key_len == 6 && strncmp(ptr, kChineseSampleText, 6) == 0) {
                            printf(" chinese 's offset is %d\n", entry_offset);
                        }
                    } 
					*/

                    //printf("%*.*s/o ", key_len, key_len, ptr);
                    //memcpy(string_pool_ptr, ptr, key_len);
                    //string_pool_ptr[key_len] = 0;
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
                    //string_pool_ptr += key_len + 1;
                }
            } // end if _term_dictionaries
        } // end for

        for(int i=0; i<MAX_PHARSE_DICTIONARY; i++) {
            if(_pharse_dictionaries[i] != NULL ) {
                entry.dict_id = i + DICTIONARY_BASE + MAX_TERM_DICTIONARY;
                for(u4 j=0; j<_pharse_dictionaries[i]->EntryCount(); j++) {
                    const char* ptr = _pharse_dictionaries[i]->GetDiskEntryByIndex(j, &key_len, &entry_offset);

                    //memcpy(string_pool_ptr, ptr, key_len);
                    //string_pool_ptr[key_len] = 0;
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
                    //string_pool_ptr += key_len + 1;
                }
            }
        }
        // all data collected.
        LOG(INFO) << "gather all terms, count is " << keys.size();
        if(bRebuildGlobalIdx)
        {
            u1 entries[1024];  // 1 for dicid 4 for each entry, 5*32 < 200 , enough
            u2 buf_len = 0;
            // build the global index.
            SafeDelete(_global_idx);
            _global_idx = new mm::DictBase();
            _global_idx->Init("com.coreseek.mm.global_idx", "entries:s"); // entries all the dict.

            mm::EntryData* entry = NULL;
            for(keymap::iterator it = keys.begin();
                    it !=  keys.end(); ++it) {
                entry = _global_idx ->Insert( it->first.c_str(), it->first.length() );
				//printf("inser key =%s, %d, %p\n", it->first, strlen(it->first), it->first);
                CHECK_NE(entry, (mm::EntryData*)NULL) << "dup key?";
                buf_len = encode_entry_in_dict(it->second, entries);
                entry->SetDataIdx(_global_idx->GetSchema(), _global_idx->GetStringPool(), 0, (const u1*)entries, buf_len);
            }
			_global_idx->BuildIndex();
        }
        // free
        keys.clear();
        free(pool);
        //free(string_pool);
    }
	LOG(INFO) << "build index done ";
    return 0;
}

int DictMgr::Reload() {
    // unload all & reload by name. ， 有可能文件已经被删除。加载前需要检查
    // 目前不提供
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
    if(!_global_idx)
        BuildIndex();
    int v = _global_idx->ExactMatch(q, len);
    if (v >= 0) {
        // get the entry's data
        mm::EntryData* entry = NULL;
        entry = _global_idx->GetEntryDataByOffset(v);
        u2 data_len = 0;
		CHECK_NE(entry, (EntryData*)NULL) << "entry is null!";
        const char* sptr = (const char*)entry->GetData(_global_idx->GetSchema(),
                                                       _global_idx->GetStringPool(), 0, &data_len);
        // decode sptr
        return decode_entry_to_matchentry((const u1*)sptr, data_len, len, rs);
    }else
        return 0; // no entry found.
}

int DictMgr::PrefixMatch(const char* q, u2 len, DictMatchResult* rs) {

    /*
     *
     */

    if(!_global_idx)
        BuildIndex();

    return 0;
}

} //mm namespace

/* -- end of file -- */
