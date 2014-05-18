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

class EntryData;

class DictSchemaColumn {
public:
    DictSchemaColumn(const char* column_name, short column_idx, char column_type)
        :_name(column_name), _idx(column_idx), _type(column_type) {
    }

public:
    inline const char* GetName() {
        return _name;
    }

    inline short GetIndex() { return _idx; }
    inline const char GetType() { return _type; }
	inline u1	 GetSize() {
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
private:
    const char* _name;
    short       _idx;
    char        _type; // name\0_type
};

class DictSchema {
public:
    DictSchema():
        _schema_define(NULL), _data_entry_size(0) {}

    virtual ~DictSchema() {
        if(_schema_define) { free(_schema_define); _schema_define = NULL; }
    }

public:
    /*
     *  The Schema Define format:
     *   <columen_name>:<char of type>[;]
     */
    int InitString(const char* schema_define);
    std::string GetColumnDefine();
    const DictSchemaColumn& GetColumn(u2 idx);
    void SetDefault(const EntryData& entry_default);
    const DictSchemaColumn* GetColumn(const char* column_name);
    u4 GetSize();   // 用于持久化, 缺少 Load & Save
    u2 GetEntryDataSize();  //定义的 EntryData 的最大尺寸, 实际根据定义 应该 u1 即可。
	u2 GetColumnCount() {   return _columns.size(); }

	// if some column missing, @return will >0, the count is the missing columns.
    // if no suite column found, will return 0; the mask used by select data, eg. select id, pinyin from dictionary ...
    int GetFieldMask(const char* columns, u2* mask);

protected:
    std::vector<DictSchemaColumn> _columns;		// use copy construct, as build schema do not care about preformace, just let it be.
    unordered_map<std::string, u2> _column_by_name;  // idx -> column name
    char*   _schema_define;
	u2		_data_entry_size;
};

} // namespace mm



#endif  //_DICTSCHEMA_H
