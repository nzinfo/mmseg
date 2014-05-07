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
#include <iostream>
#include <fstream>
#include <glog/logging.h>
#include "mm_charmap.h"

namespace mm {

////////////////////////////////////////////////////////////////////////////////
/// CharMapper: Map Unicode Char -> Unicode Char in search form.
///
/// Note:
///     Use CHECK & Exit policy because the library built interface will be called
/// by Administrators. If have some error happen, it means some hidden bug.
////////////////////////////////////////////////////////////////////////////////
const char charmap_head_mgc[] = "UNID";

typedef struct _mmseg_char_map_file_header{
    char mg[4];
    short version;
    short flags;
    u4    page_mask;    // 使用了哪几个平面
}mmseg_char_map_file_header;

#define MMSEG_CHARMAP_FLAG_DEFAULT_PASS     0x1

int
CharMapper::Load(const char* filename)
{
    /*
     * Load from disk , defined in { file_header, char[] }
     * fileheader = i4[magic_string="mmcm"], u2 ver, u2 opts[default_pass|...]
     * -404 file not found.
     * -415 file format error.
     * -416 file head is ok, file data loss.
     *
     * 0 ok
     */
    int rs = 0;
    mmseg_char_map_file_header* _header = NULL;

    LOG(INFO) << "load charmap" << filename;
    //printf("I got %s %d.\n", filename, this->_bDefaultPass);
    // legacy code, use load into memory switch only. bLoadMem = true
    u1* ptr = NULL;
    u4        ptr_length = 0;
    {
        std::ifstream ifs(filename, std::ios::binary|std::ios::ate);
        if(!ifs) {
            return CharMapper::STATUS_FILE_NOT_FOUND;
        }

        std::ifstream::pos_type pos = ifs.tellg();
        ptr_length = pos;

        ptr = new u1[ptr_length];
        ifs.seekg(0, std::ios::beg);
        ifs.read((char*)ptr, ptr_length);
        ifs.close();
    }
    /*
    csr_mmap_t * dict_file = csr_mmap_file(filename, true);
    if(!dict_file) {
        rs = -404; goto CHARMAP_LOADFAIL; }

    ptr = (const u1*)csr_mmap_map(dict_file);
    */
    // deal header
    {
        _header = (mmseg_char_map_file_header*)ptr;
        if(strncmp(charmap_head_mgc, _header->mg, 4) != 0) {
            rs = -415; goto CHARMAP_LOADFAIL; }
        // check length
        if(ptr_length < sizeof(mmseg_char_map_file_header) + sizeof(_char_mapping_base)) {
            rs = -416; goto CHARMAP_LOADFAIL; } // FIXME: should check page_mask.
        // do load , skip version now.
        this->_bDefaultPass = _header->flags & MMSEG_CHARMAP_FLAG_DEFAULT_PASS;
    }
    LOG(INFO) << "load charmap default pass is " << _bDefaultPass;

    ptr += sizeof(mmseg_char_map_file_header);

    memcpy(_char_mapping_base, ptr, sizeof(_char_mapping_base));
    // check page_mask
    if(_header->page_mask & (1<<1)) {
        ptr += sizeof(_char_mapping_base);
        memcpy(_char_mapping_page1, ptr, sizeof(_char_mapping_base));
    }
    if(_header->page_mask & (1<<2)) {
        ptr += sizeof(_char_mapping_base);
        memcpy(_char_mapping_page2, ptr, sizeof(_char_mapping_base));
    }
    if(_header->page_mask & (1<<14)) {
        ptr += sizeof(_char_mapping_base);
        memcpy(_char_mapping_page14, ptr, sizeof(_char_mapping_base));
    }

    LOG(INFO) << "load charmap done " << filename ;

CHARMAP_LOADFAIL:
    SafeDelete(ptr);
    return CharMapper::STATUS_OK;
}

int
CharMapper::Save(const char* filename)
{
    /*
     * Save to disk , defined in { file_header, char[] }
     */
    u4   _char_mapping_empty[UNICODE_PAGE_SIZE];
    memset(_char_mapping_empty, 0, sizeof(u4)*UNICODE_PAGE_SIZE);

    // mark use page[0 1 2 14]
    bool b_page0 , b_page1, b_page2, b_page14;
    b_page0 = true;
    b_page1 = b_page2 = b_page14 = false;

    mmseg_char_map_file_header header;
    header.flags = MMSEG_CHARMAP_FLAG_DEFAULT_PASS;
    header.version = 1;
    header.page_mask = 1; //only base page.
    memcpy(header.mg, charmap_head_mgc, sizeof(header.mg));
    {
        //write header
        std::FILE *fp  = std::fopen(filename, "wb");
        if(!fp)
            return -503;
        // check page
        if(memcmp(_char_mapping_empty, this->_char_mapping_page1, sizeof(_char_mapping_empty)) != 0) {
            b_page1 = true;
            header.page_mask = header.page_mask | (1<<1);
        }
        if(memcmp(_char_mapping_empty, this->_char_mapping_page2, sizeof(_char_mapping_empty)) != 0) {
            b_page2 = true;
            header.page_mask = header.page_mask | (1<<2);
        }
        if(memcmp(_char_mapping_empty, this->_char_mapping_page14, sizeof(_char_mapping_empty)) != 0) {
            b_page14 = true;
            header.page_mask = header.page_mask | (1<<14);
        }

        std::fwrite(&header,sizeof(mmseg_char_map_file_header),1,fp);
        std::fwrite(_char_mapping_base, sizeof(_char_mapping_base),1,fp);
        if(b_page1)
            std::fwrite(_char_mapping_page1, sizeof(_char_mapping_base),1,fp);
        if(b_page2)
            std::fwrite(_char_mapping_page2, sizeof(_char_mapping_base),1,fp);
        if(b_page14)
            std::fwrite(_char_mapping_page14, sizeof(_char_mapping_base),1,fp);

        std::fclose(fp);
    }
    return 0;
}

inline u4*
CharMapper::GetPage(u4 icode, u4* page_base)
{
    /*
     *
平面	始末字符值	中文名称	英文名称
0号平面	U+0000 - U+FFFF	基本多文种平面	Basic Multilingual Plane,简称BMP
1号平面	U+10000 - U+1FFFF	多文种补充平面	Supplementary Multilingual Plane,简称SMP
2号平面	U+20000 - U+2FFFF	表意文字补充平面	Supplementary Ideographic Plane,简称SIP
3号平面	U+30000 - U+3FFFF	表意文字第三平面（未正式使用[1]）	Tertiary Ideographic Plane,简称TIP
4号平面
至
13号平面	U+40000 - U+DFFFF	（尚未使用）
14号平面	U+E0000 - U+EFFFF	特别用途补充平面	Supplementary Special-purpose Plane,简称SSP
15号平面	U+F0000 - U+FFFFF	保留作为私人使用区（A区）[2]	Private Use Area-A,简称PUA-A
16号平面	U+100000 - U+10FFFF	保留作为私人使用区（B区）[2]	Private Use Area-B,简称PUA-B
    Ref: http://zh.wikipedia.org/wiki/Unicode%E5%AD%97%E7%AC%A6%E5%B9%B3%E9%9D%A2%E6%98%A0%E5%B0%84
     */
    if( (icode <= 0xFFFF) ) {
        *page_base = 0;
        return this->_char_mapping_base;
    }
    if( (0x10000 <= icode) && (icode <= 0x1FFFF) ) {
        *page_base = 0x10000;
        return this->_char_mapping_page1;
    }
    if( (0x20000 <= icode) && (icode <= 0x2FFFF) ) {
        *page_base = 0x20000;
        return this->_char_mapping_page2;
    }
    if( (0xE0000 <= icode) && (icode <= 0xEFFFF) ) {
        *page_base = 0xE0000;
        return this->_char_mapping_page14;
    }
    return NULL;
}

// mapping opt, not support A..Z/2 , should be done @ script side.
int
CharMapper::Mapping(unsigned int src, unsigned int dest, unsigned short tag)
{
    /*
     *  if entry not 0, overwrite & return 1; else 0
     */
    int rs = 0;
    CHECK_LT(src, MAX_UNICODE_CODEPOINT) << "src out of range(Unicode)!";
    CHECK_LT(dest, MAX_UNICODE_CODEPOINT) << "src out of range(Unicode)!";
    u4 base = 0;
    u4* _char_mapping = GetPage(src, &base);
    if(!_char_mapping) // should ignore
        return rs;
    src -= base;

    if(_char_mapping[src])
        rs = 1;
    _char_mapping[src] = dest | (tag<<UNICODE_BITS);
    return rs;
}

int
CharMapper::MappingRange(u4 src_begin, u4 src_end, u4 dest_begin, u4 dest_end, u2 tag)
{
    /*
     *  convert [src_begin, src_end] -> []
     *  return count of entry replaced
     *  -1 if code range length mismatch.
     */
    int rs = 0;
    // the following error should exit the program. code level logic error.
    CHECK_LT(src_begin, MAX_UNICODE_CODEPOINT) << "src(being) out of range(Unicode)!";
    CHECK_LT(dest_begin, MAX_UNICODE_CODEPOINT) << "dest(being) out of range(Unicode)!";
    CHECK_LT(src_end, MAX_UNICODE_CODEPOINT) << "src(end) out of range(Unicode)!";
    CHECK_LT(dest_end, MAX_UNICODE_CODEPOINT) << "dest(end) out of range(Unicode)!";
    CHECK_LT(src_begin, src_end) << "start should less than end!";
    CHECK_LT(dest_begin, dest_end) << "start should less than end!";
    if ( (src_end - src_begin) != (dest_end - dest_begin) )
        return -1;

    u4 base = 0;
    u4* _char_mapping = GetPage(src_begin, &base);
    if(!_char_mapping) // should ignore
        return rs;
    src_begin -= base;
    src_end -= base;

    for(u4 i = src_begin; i<=src_end; i++) {
        if(_char_mapping[i])
            rs = 1;
        _char_mapping[i] = ( dest_begin + (i-src_begin) ) | (tag<<UNICODE_BITS);
    }
    return 0;
}

int
CharMapper::MappingPass(u4 src, u2 tag)
{
    int rs = 0;
    CHECK_LT(src, MAX_UNICODE_CODEPOINT) << "src out of range(Unicode)!";

    u4 base = 0;
    u4* _char_mapping = GetPage(src, &base);
    if(!_char_mapping) // should ignore
        return rs;
    src -= base;

    if(_bDefaultPass & !tag)
        return 0; //do nothing
    if(_char_mapping[src])
        rs = 1;
    _char_mapping[src] = src | (tag<<UNICODE_BITS);
    return rs;
}

int
CharMapper::MappingRangePass(u4 src_begin, u4 src_end, u2 tag)
{
    int rs = 0;

    CHECK_LT(src_begin, MAX_UNICODE_CODEPOINT) << "src(being) out of range(Unicode)!";
    CHECK_LT(src_end, MAX_UNICODE_CODEPOINT) << "src(end) out of range(Unicode)!";
    CHECK_LT(src_begin, src_end) << "start should less than end!";

    if(_bDefaultPass & !tag)
        return 0; //do nothing

    u4 base = 0;
    u4* _char_mapping = GetPage(src_begin, &base);
    if(!_char_mapping) // should ignore
        return rs;
    src_begin -= base;
    src_end -= base;

    for(u4 i = src_begin; i<=src_begin; i++) {
        if(_char_mapping[i])
            rs = 1;
        _char_mapping[i] = i | (tag<<UNICODE_BITS);
    }
    return rs;
}

int
CharMapper::Tag(u4 src, u2 tag)
{
    CHECK_LT(src, MAX_UNICODE_CODEPOINT) << "src out of range(Unicode)!";
    u4 base = 0;
    u4* _char_mapping = GetPage(src, &base);
    if(!_char_mapping) // should ignore
        return 0;
    src -= base;

    u4 iCode = _char_mapping[src]? _char_mapping[src]:src;
    _char_mapping[src] = (iCode & UNICODE_MASK) | (tag<<UNICODE_BITS);
    return 0;
}

int
CharMapper::TagRange(u4 src_begin, u4 src_end, u2 tag)
{
    int rs = 0;
    CHECK_LT(src_begin, MAX_UNICODE_CODEPOINT) << "src(being) out of range(Unicode)!";
    CHECK_LT(src_end, MAX_UNICODE_CODEPOINT) << "src(end) out of range(Unicode)!";
    CHECK_LT(src_begin, src_end) << "start should less than end!";

    u4 base = 0;
    u4* _char_mapping = GetPage(src_begin, &base);
    if(!_char_mapping) // should ignore
        return rs;
    src_begin -= base;
    src_end -= base;
    u4 iCode = 0;

    for(u4 i = src_begin; i<=src_begin; i++) {
        if(_char_mapping[i])
            rs = 1;
        //_char_mapping[i] = i | (tag<<16);
        iCode = _char_mapping[i]? _char_mapping[i]:i;
        _char_mapping[i] = (iCode & UNICODE_MASK) | (tag<<UNICODE_BITS);
    }

    return rs;
}

u4
CharMapper::TransformScript(u4 src, u2* out_tag)
{
    return Transform(src, out_tag);
}

inline u4
CharMapper::Transform(u4 src, u2* out_tag)
{
    CHECK_LT(src, MAX_UNICODE_CODEPOINT) << "src out of range(Unicode)!";

    u4 base = 0;
    u4* _char_mapping = GetPage(src, &base);
    if(!_char_mapping) // should ignore
        return 0;
    src -= base;

    if(_char_mapping[src]) {
        if(out_tag)
            *out_tag = _char_mapping[src] >> UNICODE_BITS;
        return _char_mapping[src] & UNICODE_MASK;
    }
    // dictionary based lookup return 0.
    if(_bDefaultPass)
        return src;
    else
        return 0;
}

} // namespace mm

/* -- end of file -- */
