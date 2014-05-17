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


#if !defined(_DICTUPDATABLE_H)
#define _DICTUPDATABLE_H

#include "mm_dict_base.h"

namespace mm {

// The delta terms of DictMgr @runtime.
// if value of on term is 0, means delete.
// 
class DictUpdatable : public DictBase {
public:
	int Insert(u2 dict_id, string term);
	int Remove(u2 dict_id, string term);
	int Remove(string term);
};

} //mm namespace

#endif  //_DICTUPDATABLE_H
