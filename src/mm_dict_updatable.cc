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

#include "mm_dict_updatable.h"

namespace mm {

DictUpdatable::DictUpdatable(DictMgr* mgr) {

}

int DictUpdatable::Insert(u2 dict_id, const char* term, u2 term_len) {
    return 0;
}

int DictUpdatable::Remove(u2 dict_id, const char* term, u2 term_len) {
    return 0;
}

int DictUpdatable::Remove(const char* term, u2 term_len) {
    return 0;
}

} // namespace mm

/* -- end of file -- */
