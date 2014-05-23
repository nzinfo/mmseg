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
#include <stdlib.h>
#define  USE_CEDAR      0           // 是否使用 cedar 存储索引数据。 暂时放弃 CEDAR， 沿用保守的 DARTS.
#include <algorithm>

#include <glog/logging.h>
#include "csr_typedefs.h"
#include "csr_utils.h"
#include "mm_entry_datapool.h"
#include "mm_stringpool.h"
#include "mm_dict_base.h"

#include "utils/cedar.h"  //FIXME: license comflict. GPLv2 | LGPLv2.1
#include "utils/darts.h"

#define DICT_FLAG_IDX_DARTS         1
#define DICT_FLAG_IDX_CEDAR         2
#define DICT_FLAG_COMPRESS_ENTRY    4       // 目前版本的 entry 均不压缩， 以降低目前实现的复杂性

#define DICT_VERSION                201200u  // 目前词库文件格式的版本号

namespace mm {

const char basedict_head_mgc[] = "mmdt";    // mmseg's common dictionary. 词典格式通用，不同仅仅是 Schema 的定义，有些是预制的

class CedarDoubleArray : public cedar::da<int> {};
class DartsDoubleArray : public Darts::DoubleArray {};

class DartsEntry
{
public:
    unsigned int id;
    std::string term;
};

bool EntryAscOrderCmp (const DartsEntry& elem1, const DartsEntry& elem2 )
{
    return elem1.term < elem2.term;
}

int progress_func_darts(std::size_t a, std::size_t b) {
    printf("(%d/%d)", a, b);
    return 0;
}

DictBase::DictBase() {
    _dict_id     = 0;
    _darts_idx   = NULL;
    _cedar_idx   = NULL;
	_string_pool = NULL;
	_entry_pool = NULL;

    _entry_string2offset = NULL;
    Reset();
}

DictBase::~DictBase() {
    Reset();
    SafeDelete(_string_pool);
    SafeDelete(_entry_pool);

    SafeDelete(_darts_idx);
    SafeDelete(_cedar_idx);
    if(_entry_string2offset)
        free(_entry_string2offset);
    _entry_string2offset = NULL;
}

int DictBase::Load(const char* fname) {
    /*
     *  从文件系统加载
     */
    int rs = 0;
    std::FILE *fp = std::fopen(fname, "rb");
    if (!fp) return -1; // file not found.

    Reset();

	// init idx
	if(_darts_idx == NULL)
		_darts_idx = new DartsDoubleArray();

    // read header
    mmseg_dict_file_header header;
    std::fread(&header, sizeof(header), 1, fp);
    if(header.version == 201200)
        rs = Load_201200(fp, &header);
    std::fclose(fp);
    return rs;
}

int DictBase::Load_201200(std::FILE *fp, const mmseg_dict_file_header* header) {
    // FIXME: should check each read amount.
    u4 file_offset = sizeof(mmseg_dict_file_header);
	u4 nwrite = 0;
	// load schema
    {
        char schema[4096];
		LOG(INFO) << "read schema @"<<file_offset;
        nwrite = std::fread(schema, header->schema_size, 1, fp);
		file_offset += nwrite * header->schema_size;
        schema[header->schema_size] = 0;
        _schema.InitString(schema);

        _dict_name = header->dictname;
    }
    // load string pool
    {
        u1* ptr = (u1*)malloc(header->string_pool_size);
		LOG(INFO) << "read string pool @"<<file_offset;
        nwrite = std::fread(ptr, header->string_pool_size, 1, fp);
		file_offset += nwrite * header->string_pool_size;
        _string_pool->Load(ptr, header->string_pool_size); // the ptr's owner ship pass to string_pool , so no free here.
    }
    // load entry pool
    {
		// init entry pool
		if(_entry_pool == NULL)
			_entry_pool = new EntryDataPool(_schema.GetEntryDataSize()); 
        u1* ptr = (u1*)malloc(header->entry_pool_size);
		LOG(INFO) << "entry pool @"<<file_offset;
        nwrite = std::fread(ptr, header->entry_pool_size, 1, fp);
		file_offset += nwrite * header->entry_pool_size;
        _entry_pool->Load(ptr, header->entry_pool_size);
        _entry_count = header->entry_count;
    }
    // ignore mapping
    if(header->flags & DICT_FLAG_IDX_DARTS)
    {
        _entry_string2offset = (u4*) malloc(header->key_pair_size);
        // these data will be used when support in dict update.
		LOG(INFO) << "read(seek) idmap @"<<file_offset;
        nwrite = std::fread(_entry_string2offset, header->key_pair_size, 1, fp);
        //std::fseek(fp, header->key_pair_size, SEEK_CUR);
		file_offset += header->key_pair_size;
    }
    // load darts
    if(header->flags & DICT_FLAG_IDX_DARTS)  //FIXME: how too load cedar ?
    {
		// reset idx
		_darts_idx->clear();

        u1* ptr = (u1*)malloc(header->index_size);
		LOG(INFO) << "read darts @"<<file_offset;
        nwrite = std::fread(ptr, header->index_size, 1, fp);
		file_offset += header->index_size;
        _darts_idx->set_array(ptr, header->index_size);
    }
    return 0;
}

int DictBase::Save(const char* fname, u8 rev) {
    u4 file_offset = 0;
    u4 nwrite = 0;

    _reversion = rev;
	// check entry_pool
	if(_entry_pool == NULL) 
		return -1; // no item
    mmseg_dict_file_header header;
    memcpy(header.mg, basedict_head_mgc, 4);
    header.version = DICT_VERSION;
    header.crc32 = 0;

#if USE_CEDAR
    header.flags = DICT_FLAG_IDX_CEDAR;
#else
    header.flags = DICT_FLAG_IDX_DARTS;      //DARTS Index.
#endif

    memcpy(header.dictname, _dict_name.c_str(), _dict_name.size());
    header.dictname[_dict_name.size()] = 0;
    header.dict_rev = _reversion = rev;

    header.timestamp = currentTimeMillis();
    header.entry_count = _key2id.size();

    LOG(INFO) << "save dictionary begin " << fname;
    std::FILE *fp = std::fopen(fname, "wb");

    // deal with schema
    const std::string& schema_str = _schema.GetColumnDefine();
    header.schema_size = schema_str.length();

    // deal with string pool
    header.string_pool_size = _string_pool->GetSize();

    LOG(INFO) << "total entry count " <<  _key2id.size();
    // compress entry_pool , currently do not implement compress. leave entry uncompressed.
    header.entry_pool_size = _entry_pool->GetSize();

    /*
     * 如果后续支持 compress entry pool, 必须先压缩，再 build darts. 否则 darts 对应的 values 仍然是未压缩前的。
     */

    LOG(INFO) << "build entry index begin";
    {
        // build the darts | cedar index
        BuildIndex();
#if USE_CEDAR
        assert(0); // givme cedar index!!!
#else
		header.index_size = _darts_idx->size() * _darts_idx->unit_size();
#endif
    }
    header.key_pair_size = 0;
    if(header.flags & DICT_FLAG_IDX_DARTS)
        header.key_pair_size = ( sizeof(u4) + sizeof(i4) ) * _key2id.size();

    LOG(INFO) << "build entry index done";
    // write ...
    nwrite = std::fwrite(&header, sizeof(header), 1, fp); //header
	file_offset += sizeof(header);
	LOG(INFO) << "write schema @"<<file_offset;
    nwrite = std::fwrite(schema_str.c_str(), sizeof(char), schema_str.length(), fp); // scheam
	file_offset += sizeof(char)*schema_str.length();
	{
		u1* ptr = (u1*)malloc(header.string_pool_size);
		_string_pool->Dump(ptr, header.string_pool_size);
		LOG(INFO) << "write string pool @"<<file_offset;
		nwrite = std::fwrite(ptr, header.string_pool_size, 1, fp); // string pool
		file_offset += header.string_pool_size;
		free(ptr);
	}
    if(1)
	{
		u1* ptr = (u1*)malloc(header.entry_pool_size);
		_entry_pool->Dump(ptr, header.entry_pool_size);
		LOG(INFO) << "write entry pool @"<<file_offset;
		nwrite = std::fwrite(ptr, header.entry_pool_size, 1, fp); // entry pool
		file_offset += header.entry_pool_size;
		free(ptr);
	}

	if(header.flags & DICT_FLAG_IDX_DARTS)
	{
        // build darts's only string offset, id's mapping.
        // string offset -> id , unsort order, coder(me) is lazy...
		LOG(INFO) << "write idmap @"<<file_offset;
        for(unordered_map<std::string, u4>::iterator it = _key2id.begin();
                it !=  _key2id.end(); ++it) {
            // use a string_pool's feature, each unique string exist only one copy.
            i4 string_offset = _string_pool->AllocString(it->first.c_str(), it->first.length());
            assert(string_offset>=0);
            std::fwrite(&string_offset, sizeof(i4), 1, fp);\
            u4 entry_offset = _id2entryoffset[it->second];
            std::fwrite(&entry_offset, sizeof(u4), 1, fp);
        }// end for
		file_offset += header.key_pair_size;
	}
    // the darts part, in prev version (inner version) of mmseg, there is no dart
    if(header.flags & DICT_FLAG_IDX_DARTS)
    {
		LOG(INFO) << "write darts @"<<file_offset;
        nwrite = std::fwrite(_darts_idx->array(), _darts_idx->unit_size(), _darts_idx->size(), fp);
		file_offset += _darts_idx->size();
    }
    std::fclose(fp);
    LOG(INFO) << "save dictionary done " << fname;
    return 0;
}

const char* DictBase::GetDiskEntryByIndex(u4 idx, u2* key_len, u4* entry_offset) {
    if(!_entry_string2offset)
        return NULL;
    if(idx < EntryCount()) {
        u4 string_offset = _entry_string2offset[idx*2];
        if(entry_offset)
            *entry_offset = _entry_string2offset[idx*2+1];
        return this->_string_pool->GetString(string_offset, key_len);
    }
    return NULL;
}

int DictBase::Init(const char* dict_name, const char* schema_define) {
    Reset();
    _dict_name = dict_name;
    // check dict_name special, session, delta
    if( _dict_name == "special"
        || _dict_name == "session"
        || _dict_name == "delta")
        return -1; //invalid dict_name;

    _schema.InitString(schema_define);

    _entry_pool = new EntryDataPool(_schema.GetEntryDataSize()); // entry_data_size decided by schema.
    return 0;
}

void DictBase::Reset() {
    _schema.Reset();

    SafeDelete(_string_pool);
	SafeDelete(_entry_pool);
    _entry_pool  = NULL;
    _string_pool = new StringPoolMemory();

    //clear index
    if(_darts_idx )
        _darts_idx->clear();
    if( _cedar_idx )
        _cedar_idx->clear();

    //clear all data;
    _updatable = true;
    _reversion = 0;
    _entry_count = 0;
    // _dict_id = 0; // ???
}

/*
int DictBase::Insert(string term) {
    return 0;
}
*/

u4 DictBase::EntryCount() {
    return _entry_count;
}

u4 DictBase::BuildIndex(bool bShowProc) {
#if USE_CEDAR
    assert(0); // givme cedar index!!!
#else
    if(_darts_idx == NULL)
        _darts_idx = new DartsDoubleArray();
    _darts_idx->clear();
    // this code will be fucking slow.  // use 2min build 100M keys, seems acceptable.
    std::vector<DartsEntry> v;
	v.reserve(1024*1024*10);
    DartsEntry entry;
    for(unordered_map<std::string, u4>::iterator it = _key2id.begin();
            it !=  _key2id.end(); ++it) {
        entry.term = it->first;
        entry.id = _id2entryoffset[it->second];
        v.push_back(entry);
    }
    // sort in alphabet order
    std::sort(v.begin(), v.end(), EntryAscOrderCmp);
    // build darts's key & value pair.
    std::vector <Darts::DoubleArray::key_type *> key;
    std::vector <Darts::DoubleArray::value_type> value;
	key.reserve(1024*1024*10);
	value.reserve(1024*1024*10);

    for(std::vector<DartsEntry>::iterator it = v.begin(); it != v.end(); ++it) {
        char* ptr = &( it->term[0] );
        key.push_back(ptr);
        value.push_back(it->id);
    }
    if(bShowProc)
        return _darts_idx->build(key.size(), &key[0], 0, &value[0], progress_func_darts) ;
    else
        return _darts_idx->build(key.size(), &key[0], 0, &value[0] ) ;
#endif
}

int DictBase::ExactMatch(const char* q, u2 len) {
    /*
     * return term_offset if found. else -1 not found.
     */
#if USE_CEDAR
    assert(0); // givme cedar index!!!
#else
    if(_darts_idx == NULL)
        BuildIndex(); // assume always true.

    Darts::DoubleArray::result_pair_type  rs;
    _darts_idx->exactMatchSearch(q, rs, len);
    // check unmatch.
    return rs.value;
#endif
}

int DictBase::PrefixMatch(const char* q, u2 len, DictMatchResult* rs) {
	Darts::DoubleArray::result_pair_type result[MAX_PREFIX_SEARCH_RESULT];
	DictMatchEntry mrs;
	mrs.match._dict_id = _dict_id;
	int num = _darts_idx->commonPrefixSearch(q, result, MAX_PREFIX_SEARCH_RESULT, len);
	if(rs) {
		for(int i=0; i<num; i++) {
			mrs.match._len = result[i].length;
			mrs.match._value = result[i].value;
			rs->Match(mrs); //might be full (return -1 ), just ignore .
		}
	}
	return num;
}

int DictBase::SaveRaw(const char* fname) {
#if USE_CEDAR
    assert(0); // givme cedar index!!!
#else
    if(_darts_idx == NULL)
        BuildIndex(); // assume always true.
    _darts_idx->save(fname);
    return 0;
#endif
}

int DictBase::LoadRaw(const char* fname) {
#if USE_CEDAR
    assert(0); // givme cedar index!!!
#else
    _darts_idx->open(fname);
    return 0;
#endif
}

int DictBase::MakeUpdatable() {
    _updatable = true;  //FIXME: updatable dict is a optional feature.
    return 0;
}

bool DictBase::IsUpdatable() {
    return _updatable;
}

IStringPool* DictBase::GetStringPool() {
    return _string_pool;
}

EntryData* DictBase::Insert(const char* term, u2 len)
{
    if(_entry_pool == NULL) 
		_entry_pool = new EntryDataPool(_schema.GetEntryDataSize()); 

    /*
     * 增加新的词条
     * 1 检查是否已经存在
     *  1 在string pool 中 alloc
     *  2 新增 string offset 与 entry offset 构成 pair
     * 2 已经存在 return NULL;
     */
    // alloc in string pool, return value not used.
    i4 string_offset = _string_pool->AllocString(term, len);
    assert(string_offset>=0);

    std::string key(term, len);
    unordered_map<std::string, u4>::iterator it = _key2id.find(key);
    if(it != _key2id.end() )
        return NULL;

    CHECK_NE(_entry_pool, (EntryDataPool*)NULL) << "entry_pool is null!";

    // alloc entry
    u4 key_id = _entry_pool->NewEntryOffset();
    _key2id[key] = key_id;
    _id2entryoffset[key_id] = key_id;
    EntryData* entry = _entry_pool->GetEntry(key_id);

    CHECK_NE(entry, (EntryData*)NULL) << "entry_pool is fill? " << key_id;
    return entry;
}

EntryData* DictBase::GetEntryData(const char* term, u2 len, bool bAppendIfNotExist) {

    /*
     * 1 lookup the string -> offset
     * 2 lookup the darts index.
     */
    if(_entry_pool == NULL) return NULL;

    std::string key(term, len);
    unordered_map<std::string, u4>::iterator it = _key2id.find(key);
    if(it != _key2id.end() ) {
		// exist , lookup for offset
		u4 offset = _id2entryoffset[it->second]; // because I certanly sure about key exist in the map.
        return GetEntryDataByOffset((i4)offset);
    }
    //FIXME: check in darts.  目前不需要

    // append
    if(bAppendIfNotExist) {
        return Insert(term, len);
    }
    return NULL;
}

i4  DictBase::GetEntryOffset(const char* term, u2 len){
    if(_entry_pool == NULL) return -1;

    std::string key(term, len);
    unordered_map<std::string, u4>::iterator it = _key2id.find(key);
    if(it != _key2id.end() ) {
        // exist , lookup for offset
        return (i4)_id2entryoffset[it->second]; // because I certanly sure about key exist in the map.
    }
    return -1;
}

EntryData* DictBase::GetEntryDataByOffset(i4 term_offset) {
	if(_entry_pool == NULL) return NULL;
    return _entry_pool->GetEntry(term_offset);
}

void DictBase::SetDictionaryId(u2 dict_id_of_mgr) {
    _dict_id = dict_id_of_mgr;
}

u8 DictBase::GetReversion() {
    return _reversion;
}

} //mm namespace

/* -- end of file -- */

