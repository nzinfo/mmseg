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

namespace mm {

// The Dictionary file format:
// ------------------------------------------
// header
//  {mgc, id, rev, create_at, entry_count, schema_size, string_pool_size, entry_pool_size }
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

class DictBase {
public:
    int Load(const char* fname);
    int Save(const char* fname, u4 rev);
    int Init(const char* dict_name, const char* schema_define); // dict_name like Java's classname.
	void Reset();

    // @ return , the inner term id of the newly append string. (term_offset)
    // if - stands term pre existed.
    //int Insert(string term);

    u4 EntryCount();        // how many terms in the dictionary.

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

    IStringPool & GetStringPool();

    // return the entrydata corrosponding to the term.
    EntryData* GetEntryData(const char* term);
	// if term_offset beyone the range, a system assert will be raised.
	EntryData& GetEntryData(i4 term_offset);

	// Set the dictionary of this dicionary in the current Dictmgr, whom loaded the dictionary.
	void SetDictionaryId(u2 dict_id_of_mgr);
	u4 GetReversion();

protected:

    IStringPool* _string_pool;
	DictSchema _schema;
	EntryDataPool _entries;
    /*
	TermMap<string, u4> _terms;
	u2 _dict_id;
	vector<u4> _terms_id2entry_offset;
    */
};

} //mm namespace

#endif  //_DICTBASE_H
