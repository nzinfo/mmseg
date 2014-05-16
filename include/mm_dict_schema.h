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


#if !defined(_DICTSCHEMA_H)
#define _DICTSCHEMA_H
#include "csr_typedefs.h"
 
namespace mm {

class EntryData;

class DictSchema {
public:
	int InitString(string schema_define);
	string GetColumnDefine();
	DictSchemaColumn& GetColumn(u2 idx);
	void SetDefault(const EntryData& default);
	DictSchemaColumn& GetColumn(const char* column_name);
	u4 GetSize();
	u2 GetEntryDataSize();
	// if some column missing, @return will >0, the count is the missing columns.
	// if no suite column found, will return 0; stands 
	int GetFieldMask(string columns, u2* mask);
protected:
	vector<DictSchemaColumn> _columns;
};

} // namespace mm



#endif  //_DICTSCHEMA_H
