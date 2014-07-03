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

// FIXME: should move to dict_index
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
		nfiles = (std::min)(nfiles, MAX_TERM_DICTIONARY);
        for(int i=0; i< nfiles; i++) {
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
    return GetDictFileNames(dict_path, fext, false, files);
}

int DictMgr::GetDictFileNames(const char* dict_path, std::string fext, bool filename_only,
                            std::vector<std::string> &files)
{
    char resolved_dict_buf[255];
    files.clear();
    std::string fname; fname.reserve(255);
    std::string f_root;
    std::string f_ext;
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
          pystring::os::path::splitext(f_root, f_ext, fname);
          //if( std::string::npos != fname.rfind(fext.c_str(), fname.length() - fext.length(), fext.length()) {
          if(f_ext == fext) {
              if(filename_only)
                files.push_back(ent->d_name);
              else
                files.push_back(fname);
          }
      } // end while
      closedir (dir);
    } else {
      return 0;
    }
    return files.size();
}

DictBase* DictMgr::GetDictionary(const char* dict_name) const
{
    /*
     *  根据名字，返回词典对象， 包括 专用词表
     *  - GetDictionarySepcial 包括在内
     */
    unordered_map<std::string, mm::DictBase*>::const_iterator it = _name2dict.find(dict_name);
    if(it == _name2dict.end() )
        return NULL;
    return it->second;
}

int DictMgr::GetDictionaryID(const char* dict_name) const
{
    unordered_map<std::string, mm::DictBase*>::const_iterator it = _name2dict.find(dict_name);
    if(it == _name2dict.end() )
        return -1;
    u2 dict_id = it->second->DictionaryId();
    if(dict_id != 0)
        return dict_id;
    return -1; // no such dictionary | the dict have no id.
}

DictBase* DictMgr::GetDictionary(u2 dict_id) const
{
    /*
     *  根据 id 返回， 只能是系统的 Term & Pharse
     */
    /*
    unordered_map<u2, std::string>::const_iterator it = _id2name.find(dict_id);
    if(it == _id2name.end() )
        return NULL;
    // got a name
    return GetDictionary(it->second.c_str());
    */
    if(dict_id >= DICTIONARY_BASE && dict_id < MAX_TERM_DICTIONARY + DICTIONARY_BASE)
        return _term_dictionaries[dict_id - DICTIONARY_BASE];
    if(dict_id >= DICTIONARY_BASE+MAX_TERM_DICTIONARY
            && dict_id < DICTIONARY_BASE+MAX_TERM_DICTIONARY+MAX_PHARSE_DICTIONARY)
        return _pharse_dictionaries[dict_id - DICTIONARY_BASE+MAX_TERM_DICTIONARY];
    // FIXME: 不支持 < DICTIONARY_BASE 的查询
    return NULL;
}

const std::string DictMgr::GetDictionaryNames(const char* category) const
{
    std::ostringstream oss;
    if(strncmp(category,"term",4) == 0) {
      for(int i=0; i<MAX_TERM_DICTIONARY; i++) {
        if(_term_dictionaries[i] != NULL )
          oss<< _term_dictionaries[i]->GetDictName() <<";";
      }
    }
    return oss.str();
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
        _global_idx = new mm::DictGlobalIndex();
        _global_idx->Init("com.coreseek.mm.global_idx", GLOBAL_IDX_SCHEMA); // entries all the dict.
    }
    // checkfile.
    std::ifstream dfile(fname, std::ios::binary);
    if(!dfile)
        return -1; //file not found.
    dfile.close();
    // reopen and load.
    int nret = _global_idx->Load(fname);
    if(nret == 0) {
        _global_idx_entry_propidx = _global_idx->GetSchema()->GetColumn("entries")->GetIndex();
        _global_idx_term_tag_propidx = _global_idx->GetSchema()->GetColumn("term_tag")->GetIndex();
    }

    return nret;
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
            _term_dictionaries[i]->SetDictionaryId(DICTIONARY_BASE+i);
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
            _pharse_dictionaries[i]->SetDictionaryId(DICTIONARY_BASE+MAX_TERM_DICTIONARY+i);
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
    if(bRebuildGlobalIdx)
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
         *    use quick & dirty struct. map<std::string, dict_list>    dict_list = std::vector<dict_hit>    dict_hit = (dict_id, entry_offset)
         *  encode dict_list -> string , save as string.
         *
         *  might cause huge memory consume
         */

        keymap keys;  // 必须包括字符串的原始信息
#if HASH_MAP_C11
        keys.reserve(total_entry);
#endif

        u2 key_len = 0;
        u4 entry_offset = 0;
        // alloc all possible entryindict.
        EntryInDict* pool = (EntryInDict*) malloc( (total_entry + CSR_UINT16_MAX) * sizeof(EntryInDict) ); // 增加 UCS-2 的空间
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
            _global_idx = new mm::DictGlobalIndex();
            // FIXME: use macro | global const define global_idx's schema.
            _global_idx->Init("com.coreseek.mm.global_idx", GLOBAL_IDX_SCHEMA); // entries all the dict.

            mm::EntryData* entry = NULL;
            _global_idx_entry_propidx = _global_idx->GetSchema()->GetColumn("entries")->GetIndex();
            _global_idx_term_tag_propidx = _global_idx->GetSchema()->GetColumn("term_tag")->GetIndex();
            /*
             * 词典补完：
             * 1 如果在 UCS-2 空间中，有字符不再 keys 中，则 增加； term_tag 与 script_type 同；
             *   对应的 EntryInDict 为 NULL
             * 2 检测每个词条中，每个字的类型。判断词条的基本类型
             *   - 默认为中文
             *   - 如果全部为数字，则标记为数字
             *   - 如果全部为符号，则标记为符号
             *   - 如果全部为字母，则标记为西文
             *   - [目前不实现] 如果存在某个 dict ，指定了 term_tag，则从其指定
             *   - 对于不在词典中的新出现的词条，取其第一个字符长度 tag
             */

            {
                keymap::const_iterator cit;
                char char_utf8[10];
                for(u4 icode = 1; icode <= CSR_UINT16_MAX; icode++) {
                    // encode icode -> utf8
                    char_utf8[csr::csrUTF8Encode((u1*)char_utf8, icode)] = 0;  //必然可以被正确的编码
                    // query in key
                    cit = keys.find(char_utf8);
                    if(cit==keys.end()) {
                        // 因为其他算法模块的原因， 此处不能为 NULL
                        // icode missing
                        // keys[char_utf8] = NULL;
						pool_ptr->dict_id = 0;
                        pool_ptr->entry_offset = 0;
                        pool_ptr->next = NULL;
                        keys[char_utf8] = pool_ptr;
                        pool_ptr ++;

                    }
                } // end for each icode
            }

            // 确定 CJK 区的 Tag
            u2 _cjk_chartag = 0;
            u2 _num_chartag = 0;
            u2 _ascii_chartag = 0;
            u2 _sym_chartag = 0;
            u2 char_tag = 0;
            u2 term_tag = 0;
            {
                GetCharMapper()->Transform( (u4)0x4E2D, &_cjk_chartag );  // Chinese Char `中`
                GetCharMapper()->Transform( (u4)'1', &_num_chartag );
                GetCharMapper()->Transform( (u4)'A', &_ascii_chartag );
                GetCharMapper()->Transform( (u4)',', &_sym_chartag );
            }


            for(keymap::iterator it = keys.begin();
                    it !=  keys.end(); ++it) {
                entry = _global_idx ->Insert( it->first.c_str(), it->first.length() );
                //printf("inser key =%s, %d, %p\n", it->first, strlen(it->first), it->first);
                CHECK_NE(entry, (mm::EntryData*)NULL) << "dup key?";
                if(it->second) {
                    buf_len = encode_entry_in_dict(it->second, entries);
                    entry->SetDataIdx(_global_idx->GetSchema(), _global_idx->GetStringPool(),
                                      _global_idx_entry_propidx, (const u1*)entries, buf_len);
                }

                {
                    // 检查词条的类别
                    // 1 解码回 icode
                    // 检查编码是否都一样
                    // 检查是否是 _num_chartag  _ascii_chartag _sym_chartag
                    // 标注 词条
                    term_tag = _cjk_chartag;

                    const char* ptr_begin = it->first.c_str();
                    const char* ptr_end = ptr_begin + it->first.length();
                    const char* ptr = ptr_begin;

                    u2 utf8_icode_len = 0;
                    int iCode = 0;
                    while(*ptr && ptr < ptr_end) {
                        iCode = csr::csrUTF8Decode (
                                    (const u1*)ptr, utf8_icode_len);
                        // query icode's type
                        ptr += utf8_icode_len;
                        GetCharMapper()->Transform( (u4)iCode, &char_tag );
                        if (term_tag == 0)
                            term_tag = char_tag;
                        if (term_tag != char_tag)
                            term_tag = CSR_UINT16_MAX; //设置为最大的u2, 表示无效。 自动检测只能检查字符都属于同一个 script_type 的情况。
                    } //end while check

                    if(term_tag == CSR_UINT16_MAX)
                        term_tag = _cjk_chartag;
                    //注册之
                    entry->SetU2(_global_idx->GetSchema(), _global_idx_term_tag_propidx, term_tag);
                }
            }
            _global_idx->BuildIndex();
        }
        // free
        keys.clear();
        free(pool);
        //free(string_pool);
    }

    // 遍历全部词典，确定 能够提供哪些 fields
    {
        unordered_map<std::string, BaseDictColumnReadMarkerList>::iterator field_it;
        for(int i=0; i<MAX_TERM_DICTIONARY; i++) {
            if(_term_dictionaries[i] != NULL ) {
                const mm::DictSchema* schema = _term_dictionaries[i]->GetSchema();
                for(u2 col_i = 0; col_i < schema->GetColumnCount(); col_i++) {
                    BaseDictColumnReadMarker marker;
                    marker.dict_id = _term_dictionaries[i]->DictionaryId();
                    marker.column_datatype = schema->GetColumn(col_i).GetType();
                    marker.prop_dict_idx = schema->GetColumn(col_i).GetIndex();

                    // 1 check column is pre-existed.
                    field_it = _fields.find(schema->GetColumn(col_i).GetName());
                    if(field_it == _fields.end()) {
                        // append new
                        BaseDictColumnReadMarkerList list;
                        list.push_back(marker);
                        // 2 add access path
                        _fields[schema->GetColumn(col_i).GetName()] = list;
                    }else{
                        // reuse existed.
                        // 2 add access path
                        field_it->second.push_back(marker);
                    }
                } // for each columns
            }
        }
        // 理论上， pharse 应该没有 property 了。
        for(int i=0; i<MAX_PHARSE_DICTIONARY; i++) {
            if(_pharse_dictionaries[i] != NULL ) {
                const mm::DictSchema* schema = _pharse_dictionaries[i]->GetSchema();
                for(u2 col_i = 0; col_i < schema->GetColumnCount(); col_i++) {
                    BaseDictColumnReadMarker marker;
                    marker.dict_id = _term_dictionaries[i]->DictionaryId();
                    marker.column_datatype = schema->GetColumn(col_i).GetType();
                    marker.prop_dict_idx = schema->GetColumn(col_i).GetOffset();

                    // 1 check column is pre-existed.
                    field_it = _fields.find(schema->GetColumn(col_i).GetName());
                    if(field_it == _fields.end()) {
                        // append new
                        BaseDictColumnReadMarkerList list;
                        list.push_back(marker);
                        // 2 add access path
                        _fields[schema->GetColumn(col_i).GetName()] = list;
                    }else{
                        // reuse existed.
                        // 2 add access path
                        field_it->second.push_back(marker);
                    }
                } // for each columns
            }
        }
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

int DictMgr::ExactMatch(const char* q, u2 len, mm::DictMatchResult* rs) {
    if(!_global_idx)
        BuildIndex();
    int v = _global_idx->ExactMatch(q, len, rs);
    return v;
}

int DictMgr::PrefixMatch(const char* q, u2 len, mm::DictMatchResult* rs) {

    /*
     *  返回全局范围内，符合条件的词条。不保证长度的严格有序。
     */

    if(!_global_idx)
        BuildIndex();
    int num = _global_idx->PrefixMatch(q, len, rs);
    return num;
}

int DictMgr::ExactMatch(u4* q, u2 len, mm::DictMatchResult* rs)
{
    // FIXME: impl icode match.
    assert(0);
    return 0;
}
int DictMgr::PrefixMatch(u4* q, u2 len, mm::DictMatchResult* rs, bool extend_value)
{
    if(!_global_idx)
        BuildIndex();
    int num = _global_idx->PrefixMatch(q, len, rs, extend_value);
    return num;
}

int DictMgr::GetMatchByDictionary(const mm::DictMatchEntry* mentry, u2 term_len, mm::DictMatchResult* rs) const
{
    /*
     * 使用 _global_idx 返回的 entry, 展开结果集
     */
    mm::EntryData* entry = NULL;
    u2 data_len = 0;

    entry = _global_idx->GetEntryDataByOffset(mentry->match._value);
    const char* sptr = (const char*)entry->GetData( _global_idx->GetSchema(),
                                                    _global_idx->GetStringPool(), _global_idx_entry_propidx, &data_len);
    return decode_entry_to_matchentry((const u1*)sptr, data_len, term_len, rs);
}

} //mm namespace

/* -- end of file -- */
