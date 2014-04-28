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

/*
 * Provider a simple & stupid StringPool
 */

#if !defined(_ISTRINGPOOL_H)
#define _ISTRINGPOOL_H
#include "csr_typedefs.h"

namespace mm {

// Gather all string into one memory block, so gain more compress ratio.
// Ensure each unique string copy have only one instance.
class IStringPool {
public:
	// input a buffer of string, return the offset of the string in this string pool.
	// The string pool format
	// length:u4, data:u1
	// 
	virtual i4 AllocString(const char* buf, u2 length) = 0;
	// return the string's ptr by string 's offset
    virtual const char* GetString(u4 offset, u2* data_length) = 0;
	virtual u4 GetSize() = 0;

    virtual int Dump(u1* ptr, u4 size) = 0;
    virtual int Load(u1* ptr, u4 size) = 0;
};

} // namespace mm


#endif  //_ISTRINGPOOL_H
