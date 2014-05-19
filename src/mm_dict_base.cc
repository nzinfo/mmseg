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

#define DICT_FLAG_IDX_DARTS         0
#define DICT_FLAG_IDX_CEDAR         1
#define DICT_FLAG_COMPRESS_ENTRY    2       // 目前版本的 entry 均不压缩， 以降低目前实现的复杂性

namespace mm {

const char basedict_head_mgc[] = "mmdt";    // mmseg's common dictionary. 词典格式通用，不同仅仅是 Schema 的定义，有些是预制的

typedef struct _mmseg_dict_file_header{
    char mg[4];
    short version;              // 词库文件的版本号，目前统一为 2012
    short flags;                // 标记 darts 索引的类型； darts ? cedar ?; entry 数据是否压缩
    char dictname[128];         // 对应词典的名称
    u8   dict_rev;              // dictionay reversion.
    u8   timestamp;             // create_at ?
    u4   entry_count;           // 多少词条
    u4   schema_size;           // Schema 字符串定义的尺寸
    u4   string_pool_size;      // 词库中用到的字符串长度的尺寸
    u4   entry_pool_size;       // 词条的属性数据的存储空间占用
    u4   key_pair_size;         // 当使用 darts 时， 用于记录原始的 key-> entry_offset 的关系  == sizeof(u4+u4)*entry_count, 如果为 0, 则必须为 cedar
    u4   index_size;            // darts 索引占用的的空间
    u4   crc32;                 // 整个文件，除头部外的校验码。 目前保留为 0.
}mmseg_dict_file_header;

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

DictBase::DictBase() {
    _dict_id     = 0;
    _darts_idx   = NULL;
    _cedar_idx   = NULL;
    Reset();
}

DictBase::~DictBase() {
    Reset();
    SafeDelete(_string_pool);
    SafeDelete(_entry_pool);

    SafeDelete(_darts_idx);
    SafeDelete(_cedar_idx);
}

int DictBase::Load(const char* fname) {
    return 0;
}

int DictBase::Save(const char* fname, u8 rev) {
    _reversion = rev;

    mmseg_dict_file_header header;
    memcpy(header.mg, basedict_head_mgc, 4);
    header.version = 2012;
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
    std::FILE *fp = std::fopen(fname, "rb");

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
#if USE_CEDAR
        assert(0); // givme cedar index!!!
#else
        if(_darts_idx == NULL)
            _darts_idx = new DartsDoubleArray();
        // this code will be fucking slow.
        std::vector<DartsEntry> v;
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
		for(std::vector<DartsEntry>::iterator it = v.begin(); it != v.end(); ++it) {
			char* ptr = &( it->term[0] );
			key.push_back(ptr);
			value.push_back(it->id);
		}
        _darts_idx->build(key.size(), &key[0], 0, &value[0] ) ;
#endif
    }
    header.entry_pool_size = _darts_idx->size();

    LOG(INFO) << "build entry index done";

    // write darts...
    std::fclose(fp);
    LOG(INFO) << "save dictionary done " << fname;
    return 0;
}

int DictBase::Init(const char* dict_name, const char* schema_define) {
    Reset();
    _dict_name = dict_name;
    _schema.InitString(schema_define);

    _entry_pool = new EntryDataPool(_schema.GetEntryDataSize()); // entry_data_size decided by schema.
    return 0;
}

void DictBase::Reset() {
    _schema.Reset();

    SafeDelete(_string_pool);
    _entry_pool  = NULL;
    _string_pool = new StringPoolMemory();

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

u4 DictBase::BuildIndex() {
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
    _updatable = true;
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
    if(_entry_pool == NULL) return NULL;
    /*
     * 增加新的词条
     * 1 检查是否已经存在
     *  1 在string pool 中 alloc
     *  2 新增 string offset 与 entry offset 构成 pair
     * 2 已经存在 return NULL;
     */
    std::string key(term, len);
    unordered_map<std::string, u4>::iterator it = _key2id.find(key);
    if(it != _key2id.end() )
        return NULL;

    // alloc entry
    u4 key_id = _entry_pool->NewEntryOffset();
    _key2id[key] = key_id;
    _id2entryoffset[key_id] = key_id;
    return _entry_pool->GetEntry(key_id);
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
		return GetEntryData((i4)offset);
    }
    //FIXME: check in darts.

    // append
    if(bAppendIfNotExist) {
        return Insert(term, len);
    }
    return NULL;
}


EntryData* DictBase::GetEntryData(i4 term_offset) {
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

