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
#include <sstream>

#include "mm_entrydata.h"
#include "mm_dict_schema.h"

#include "utils/pystring.h"

namespace mm {

int DictSchema::InitString(const char* schema_define) {

    size_t str_define_len = strlen(schema_define);
    _schema_define = (char*)malloc(str_define_len+1);

    memcpy(_schema_define, schema_define, str_define_len);
    _schema_define[str_define_len] = 0;

    // do not consider performance here.
    {
		char column_type = '\0';
        u4 column_len = 0;
        std::vector<std::string> column_exprs;
        pystring::split(_schema_define, column_exprs, ";");
        for(std::vector<std::string>::iterator it = column_exprs.begin(); it != column_exprs.end(); ++it) {
			// check type
			column_type = _schema_define[column_len + it->length() -1];
			bool is_valid_type =  ( column_type == '2' ||  column_type == '4' || column_type == '8' || column_type == 's');
			CHECK(is_valid_type) << "invalid column type " << column_type;

            DictSchemaColumn column(_schema_define + column_len, _columns.size(), column_type);
            column_len += it->length();
			_schema_define[column_len-2] = 0;
			column_len ++ ; //skip ';'
            _columns.push_back(column);
			// build by name
			_column_by_name[column.GetName()] = _columns.size() - 1; // record the offset in _columns
        }
		// FIXME: return an error code ?
		CHECK_LE(_columns.size(), MAX_COLUMN_COUNT) << "column count larger than max column count. " << MAX_COLUMN_COUNT;
    }
	
	_data_entry_size  = 0; // update entry's size.
	for(std::vector<DictSchemaColumn>::iterator it = _columns.begin();
		it != _columns.end(); ++it) {
            it->SetOffset(_data_entry_size + sizeof(u2)); // the offset include mask. so that 0 can mark as a mistake.
			_data_entry_size += it->GetSize();
	}

    // pollute mask_offset_lookup. might slow...
    {
        // max execute time 32768 * 15
        memset(_mask_offset_lookup, 0, sizeof(_mask_offset_lookup));
        for(int i=0; i< ( 1<< _columns.size() ); i++ ) {
            // enum every mask condition might meet.
            u2 mask = i;
            for(u1 j = 0; j<15; j++) {
                if( (mask>>j) & 1) {
                    _mask_offset_lookup[i] += _columns[j].GetSize();
                }
            } //end for j
        } // end for i
    }

    if(DEBUG_MM_SCHEMA)
    {
        for(std::vector<DictSchemaColumn>::iterator it = _columns.begin();
            it != _columns.end(); ++it) {
            printf("tok=%s, type=%c\t", it->GetName(), it->GetType() );
        }
    }
    return 0;
}

std::string DictSchema::GetColumnDefine() {
    std::ostringstream oss;
    for(std::vector<DictSchemaColumn>::iterator it = _columns.begin();
        it != _columns.end(); ++it) {
        oss<<it->GetName()<<":"<<it->GetType()<<";";
    }
    return oss.str();
}

const DictSchemaColumn& DictSchema::GetColumn(u2 idx) const {
	// check is in columns
    CHECK_LT(idx, _columns.size()) << "no such column @ position " << idx;
    return _columns[idx];
}

/*
void DictSchema::SetDefault(const EntryData& entry_default) {

}
*/

const DictSchemaColumn* DictSchema::GetColumn(const char* column_name) const {
	/*
	 * 理论上， 如果增加了新的 Column， 则这个 pointer 是不稳定的。 但是实际设计中， column 一次定义，不再修改。
	 */
	unordered_map<std::string, u2>::const_iterator it = _column_by_name.find(column_name);
	if(it != _column_by_name.end() ) {
		return & (_columns[it->second]);
	}
	return NULL;
}

u4 DictSchema::GetSize() {
	/*
	 * 计算对 Schema 持久化需要的空间
	 *  schema_define + default_data_entry.
	 *  不启动 default_data_entry, 整个 entry 池中， 第 0 个 entry 即为 default value.
	 */
	return GetColumnDefine().length() + 2; // with 2 byte save the string's length.
}

u2 DictSchema::GetEntryDataSize() {
    return _data_entry_size;
}

int DictSchema::GetFieldMask(const char* columns, u2* mask) {
	/*
	 * 此处与类型定义不同， 只需要传入 FieldName 即可
	 *  eg. id;pinyin;thres
	 */ 
	u2 column_mask = 0;
	int column_missing = 0;
	{
		std::vector<std::string> column_exprs;
		pystring::split(columns, column_exprs, ";");
		unordered_map<std::string, u2>::iterator column_it;

		for(std::vector<std::string>::iterator it = column_exprs.begin(); it != column_exprs.end(); ++it) {
			 column_it = _column_by_name.find(*it);
			 if(column_it != _column_by_name.end() ) {
				 // column mask order is <-  column14, column13, ... column0
				 column_mask = column_mask | (1 << _columns[column_it->second].GetIndex() );
				 printf("mask %d idx=%d\n", column_mask, _columns[column_it->second].GetIndex() );
			 }else{
				 column_missing ++;
			 }
		}
		if(mask)
			*mask = column_mask;
	}
	return column_missing;
}

} // namespace mm

/* -- end of file -- */
