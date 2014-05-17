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

#if !defined(_DICTSCHEMACOLUMN_H)
#define _DICTSCHEMACOLUMN_H

namespace mm {

class DictSchemaColumn {
public:
    DictSchemaColumn(const char* column_name, short column_idx, char column_type)
        :_name(column_name), _idx(column_idx), _type(column_type){

    }

public:
    inline const char* GetName() {
        return _name;
    }

    inline short GetIndex() { return _idx; }
    inline const char GetType() { return _type; }
private:
    const char* _name;
    short       _idx;
    char        _type;
};

} // namespace mm
#endif  //_DICTSCHEMACOLUMN_H
