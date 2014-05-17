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

#if !defined(_DICTMGR_H)
#define _DICTMGR_H

namespace mm {

#include "DictUpdatable.h"

// The global term index's format.
// 
// key, value (offset)
// 
// data = dictmask (4u), values of the term in other dicts.
// 
// {
//      1 foreach in dicts
//      2 is the dict have such property?
//      3 entry have such property ?
// }
class DictMgr {
public:
	DictTerm* term_dictionaries;
	DictUpdatable delta_dictionary;
	DictPharse* pharse_dictionaries;
	int Load(string dict_path);
	DictBase& GetDictionary(string dict_name);
	DictBase& GetDictionary(u2 dict_id);
	u2 GetDictionaryIdx(string dict_name);
	// Build 
	int LoadIndexCache(string fname);
	int SaveIndexCache(string fname);
	int BuildIndex();
	int Reload();
	void UnloadAll();
	int VerifyIndex();
	// As we have many dictionary, each diction might contain same term.
	// Even a single term might case multi hit-entry, so we needs MatchResult here.
	int ExactMatch(const char* q, u2 len, DictMatchResult* rs);
	int PrefixMatch(const char* q, u2 len, DictMatchResult* rs);
	DictUpdatable& GetUpdatableDictionary();
};

} // namespace mm

#endif  //_DICTMGR_H
