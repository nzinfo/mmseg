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


#if !defined(_DICTTERMUSER_H)
#define _DICTTERMUSER_H

#include "mm_dict_base.h"
#include "mm_dict_term.h"

namespace mm {

// save session & ctx related terms.
// might be transferred via web.
class DictTermUser : public DictTerm {
public:
    std::string DumpAsJSON();
    int LoadFromJSON(std::string s);
};

} // namespace mm

#endif  //_DICTTERMUSER_H
