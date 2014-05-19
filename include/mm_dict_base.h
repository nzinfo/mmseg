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
 *
 */

#if !defined(_DICTBASE_H)
#define _DICTBASE_H

#include "mm_dict_schema.h"
#include "iface_stringpool.h"
#include "mm_entry_datapool.h"
#include "mm_match_result.h"
#include "mm_hashmap.h"
#include "mm_entrydata.h"

//FIXME: move the define -> some common header.
#define SafeDelete(_arg)		{ if ( _arg ) delete ( _arg );		(_arg) = NULL; }
#define SafeDeleteArray(_arg)	{ if ( _arg ) delete [] ( _arg );	(_arg) = NULL; }

namespace mm {

// The Dictionary file format:
// ------------------------------------------
// header
//  {mgc, id, rev, create_at, entry_count, schema_size, string_pool_size, entry_pool_size, darts_index_size }
// schema
//   string of schema define; default entry value.
// string pool
//   list of (length, data)
// entry pool
//   list of (mask, data) , once you have mask, u've gotten length.
// key_offset_list 
//   [u4 of string pool's offset, u4 of entry pool's offset]
// 
// There is a compile time flags to decide which darts library will be used.
// 
// 
// _terms: 记录实际的词条 和 本词条是第几个 term
// _terms_id2entry_offset: 记录 词条编号 到 entryDataPool 的偏移量编号， 不同的模式，该编号不同。
class DartsDoubleArray;
class CedarDoubleArray;

class DictBase {
public:
    DictBase();
    virtual ~DictBase();
public:
    int Load(const char* fname);
    int Save(const char* fname, u8 rev);
    int Init(const char* dict_name, const char* schema_define); // dict_name like Java's classname.
	void Reset();

    // @ return , the inner term id of the newly append string. (term_offset)
    // if - stands term pre existed.
    //int Insert(string term); if return NULL, term existed.
    EntryData* Insert(const char* term, u2 len);

    u4 EntryCount();        // how many terms in the dictionary.

    u4 BuildIndex();
	// Find the exactly term in the diictionary.
	// if found,  return the offset of term.
	// if not, return ERR_TERM_NOT_FOUND
	int ExactMatch(const char* q, u2 len);
	// return all entry share the same prefix.
	// if rs is NULL, just return the count of result in prefix matching.
	int PrefixMatch(const char* q, u2 len, DictMatchResult* rs);

    // Save Raw Darts into file, used for debug only.
    int SaveRaw(const char* fname);
    int LoadRaw(const char* fname);

    int MakeUpdatable();
	bool IsUpdatable();

    IStringPool* GetStringPool();
    const DictSchema* GetSchema() const { return &_schema; }

    // return the entrydata corrosponding to the term.
    EntryData*   GetEntryData(const char* term, u2 len, bool bAppendIfNotExist = false);
    i4           GetEntryOffset(const char* term, u2 len);
	// if term_offset beyone the range, a system assert will be raised.
    EntryData*   GetEntryData(i4 term_offset);

	// Set the dictionary of this dicionary in the current Dictmgr, whom loaded the dictionary.
	void SetDictionaryId(u2 dict_id_of_mgr);
    u8 GetReversion();

protected:

    IStringPool*	_string_pool;
	DictSchema		_schema;
	EntryDataPool*	_entry_pool;

    bool            _updatable;
    u2              _dict_id;
    std::string     _dict_name;
    u8              _reversion;
    u4              _entry_count;
    /*
     *  id 为 term 加入时的 entry_offset。 引入 id 的原因在于 entry_offset 会随着 entry_pool 的压缩而发生变化。
     */
    unordered_map<std::string, u4> _key2id;
    unordered_map<u4, u4> _id2entryoffset;

    // darts like index.
    DartsDoubleArray* _darts_idx;
    CedarDoubleArray* _cedar_idx;
};

} //mm namespace

#endif  //_DICTBASE_H
