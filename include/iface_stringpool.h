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
 * THIS SOFTWARE IS PROVIDED BY CYBOZU LABS, INC. ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL CYBOZU LABS, INC. OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Cybozu Labs, Inc.
 *
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
	virtual u4 AllocString(const char* buf, u2 length) = 0;
	// return the string's ptr by string 's offset
	virtual const char* GetString(u4 offset, u4* data_length) = 0;
	virtual u4 GetSize() = 0;
};

} // namespace mm

#endif  //_ISTRINGPOOL_H
