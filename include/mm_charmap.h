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

/*
 *  Char mapping: map a unicode char -> an other similary form.
 *
 */
#ifndef MM_CHARMAP_H
#define MM_CHARMAP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "csr_typedefs.h"

// CharMapper define
#define MAX_UNICODE_CODEPOINT   0x10FFFFu      // Unicode Max
#define UNICODE_MASK            0x1FFFFFu      // (2**21-1)
#define UNICODE_BITS            21u            // Unicode's bits count.
#define UNICODE_PAGE_SIZE       0xFFFFu        // 65536

#define SafeDelete(_arg)		{ if ( _arg ) delete ( _arg );		(_arg) = NULL; }
#define SafeDeleteArray(_arg)	{ if ( _arg ) delete [] ( _arg );	(_arg) = NULL; }

namespace mm {

class CharMapper
{
    /*
     * Convert char -> simp | sym case. eg. A->a; 0xF900 -> 0x8c48
     * Get char's category number. ascii...
     *
     * 256K in memory by default
     * Ref: http://www.fmddlmyy.cn/text17.html
     *      http://www.unicode.org/Public/UNIDATA/
     *          = Scripts.txt
     */
public:
    CharMapper(bool default_pass=false):_bDefaultPass(default_pass) {
        memset(_char_mapping_base, 0, sizeof(u4)*UNICODE_PAGE_SIZE);
        memset(_char_mapping_page1, 0, sizeof(u4)*UNICODE_PAGE_SIZE);
        memset(_char_mapping_page2, 0, sizeof(u4)*UNICODE_PAGE_SIZE);
        memset(_char_mapping_page14, 0, sizeof(u4)*UNICODE_PAGE_SIZE);
    }

public:
    int Load(const char* filename);
    int Save(const char* filename);

    // mapping opt, not support A..Z/2 , should be done @ script side.
    int Mapping(u4 src, u4 dest, u2 tag = 0);
    int MappingRange(u4 src_begin, u4 src_end, u4 dest_begin, u4 dest_end, u2 tag = 0);
    int MappingPass(u4 src_begin, u2 tag = 0);
    int MappingRangePass(u4 src_begin, u4 src_end, u2 tag = 0);

    int Tag(u4 icode, u2 tag);
    int TagRange(u4 src_begin, u4 src_end, u2 tag);

    inline u4 Transform(u4 src, u2* out_tag);
    u4 TransformScript(u4 src, u2* out_tag);  // the script side , check for trans

protected:
    inline u4* GetPage(u4 icode, u4 *page_base);

private:
    //FIXME: cut down memory consume.
    u4   _char_mapping_base[UNICODE_PAGE_SIZE];
    u4   _char_mapping_page1[UNICODE_PAGE_SIZE];
    u4   _char_mapping_page2[UNICODE_PAGE_SIZE];
    u4   _char_mapping_page14[UNICODE_PAGE_SIZE];   // page 15, page 16 will be ignored.
    //u4   _char_mapping[MAX_UNICODE_CODEPOINT]; // lower bit is trans iCode, higher is category flag.  16M, we run code only on server right ?
    bool _bDefaultPass;

public:
    // function status code
    static int STATUS_OK;
    static int STATUS_FILE_NOT_FOUND;
};

} // namespace mm

#endif // CHARMAP_H

/* Should provide a script build char mapping .*/
/*
# The expected value format is a commas-separated list of mappings.
# Two simplest mappings simply declare a character as valid, and map a single character
# to another single character, respectively. But specifying the whole table in such
# form would result in bloated and barely manageable specifications. So there are
# several syntax shortcuts that let you map ranges of characters at once. The complete
# list is as follows:
#
# A->a
#     Single char mapping, declares source char 'A' as allowed to occur within keywords
#     and maps it to destination char 'a' (but does not declare 'a' as allowed).
# A..Z->a..z
#     Range mapping, declares all chars in source range as allowed and maps them to
#     the destination range. Does not declare destination range as allowed. Also checks
#     ranges' lengths (the lengths must be equal).
# a
#     Stray char mapping, declares a character as allowed and maps it to itself.
#     Equivalent to a->a single char mapping.
# a..z
#     Stray range mapping, declares all characters in range as allowed and maps them to
#     themselves. Equivalent to a..z->a..z range mapping.
# A..Z/2
#     Checkerboard range map. Maps every pair of chars to the second char.
#     More formally, declares odd characters in range as allowed and maps them to the
#     even ones; also declares even characters as allowed and maps them to themselves.
#     For instance, A..Z/2 is equivalent to A->B, B->B, C->D, D->D, ..., Y->Z, Z->Z.
#     This mapping shortcut is helpful for a number of Unicode blocks where uppercase
#     and lowercase letters go in such interleaved order instead of contiguous chunks.
*/
