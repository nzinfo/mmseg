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


#if !defined(_DICTSCHEMA_H)
#define _DICTSCHEMA_H
#include <string>
#include <vector>
#include "csr_typedefs.h"
#include "mm_hashmap.h"

#define MAX_COLUMN_NAME_LENGTH 64u  // 词库中， 字段名称的最大长度
#define MAX_COLUMN_COUNT	   15u  // the highest field use as a flag of compress or not.

// Debug Schema
#define DEBUG_MM_SCHEMA 1

namespace mm {

struct EntryData;

class DictSchemaColumn {
public:
    DictSchemaColumn(const char* column_name, short column_idx, char column_type)
        :_name(column_name), _idx(column_idx), _type(column_type) {
        _uncompress_offset = 0;
    }

public:
    inline const char* GetName() {
        return _name;
    }

    inline short GetIndex() const { return _idx; }
    inline const char GetType() const { return _type; }
    inline u1	 GetSize() const {
		switch(_type) {
		case '2':
			return 2;
		case '4':
			return 4;
		case '8':
			return 8;
		case 's':
			return 4;	// the offset in the string pool.
		}
		// unsupported type.
		return 0;
	}
    inline void SetOffset(u2 u) { _uncompress_offset = u; }
    inline u2   GetOffset() const { return _uncompress_offset; }

private:
    const char* _name;
    short       _idx;
    char        _type; // name\0_type
    u2          _uncompress_offset;
};

class DictSchema {
public:
    DictSchema():
        _schema_define(NULL), _data_entry_size(0) {}

    virtual ~DictSchema() {
        if(_schema_define) { free(_schema_define); _schema_define = NULL; }
    }


	void Reset() {
		if(_schema_define) { free(_schema_define); _schema_define = NULL; }
		_data_entry_size = 0;
		_columns.clear();
		_column_by_name.clear();
	}
public:
    /*
     *  The Schema Define format:
     *   <columen_name>:<char of type>[;]
     */
    int InitString(const char* schema_define);
    std::string GetColumnDefine();
    const DictSchemaColumn& GetColumn(u2 idx) const;
    void SetDefault(const EntryData& entry_default);
    const DictSchemaColumn* GetColumn(const char* column_name) const;
    u4 GetSize();   // 用于持久化, 缺少 Load & Save
    u2 GetEntryDataSize();  //定义的 EntryData 的最大尺寸, 实际根据定义 应该 u1 即可。
	u2 GetColumnCount() const {   return _columns.size(); } 

	// if some column missing, @return will >0, the count is the missing columns.
    // if no suite column found, will return 0; the mask used by select data, eg. select id, pinyin from dictionary ...
    int GetFieldMask(const char* columns, u2* mask);
    inline u2  GetCompressedOffset(u2 idx, u2 mask) const {
        /*
         *  调用本函数之前，需要保证 idx 对应的 mask 位 有值。如果没有, 返回 0
         */
        u2 offset_idx = ((1<<idx) - 1) & mask;
        return _mask_offset_lookup[offset_idx];
    }

protected:
    std::vector<DictSchemaColumn> _columns;		// use copy construct, as build schema do not care about preformace, just let it be.
    unordered_map<std::string, u2> _column_by_name;  // idx -> column name
    char*   _schema_define;
	u2		_data_entry_size;

    // 用于压缩存储 EntryData 时，快速计算 offset 的速查表。因为最多 15个字段， 最长 32768, 1char
    u1 _mask_offset_lookup[32768];
};

} // namespace mm



#endif  //_DICTSCHEMA_H
