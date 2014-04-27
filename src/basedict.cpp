#include <glog/logging.h>

#include "basedict.h"

namespace mm {

////////////////////////////////////////////////////////////////////////////////
/// CharMapper: Map Unicode Char -> Unicode Char in search form.
///
/// Note:
///     Use CHECK & Exit policy because the library built interface will be called
/// by Administrators. If have some error happen, it means some hidden bug.
////////////////////////////////////////////////////////////////////////////////
int
CharMapper::Load(const char* filename)
{
    /*
     * Load from disk , defined in { file_header, char[] }
     *
     * -404 file not found.
     * -4xx file format error.
     *
     * 200 ok
     */
    printf("I got %s %d.\n", filename, this->_bDefaultPass);

    return 0;
}

int
CharMapper::Save(const char* filename)
{
    /*
     * Save to disk , defined in { file_header, char[] }
     */
    return 0;
}

// mapping opt, not support A..Z/2 , should be done @ script side.
int
CharMapper::Mapping(unsigned int src, unsigned int dest, unsigned short tag)
{
    /*
     *  if entry not 0, overwrite & return 1; else 0
     */
    int rs = 0;
    CHECK_LT(src, MAX_UNICODE_CODEPOINT) << "src out of range(UCS-2)!";
    CHECK_LT(dest, MAX_UNICODE_CODEPOINT) << "src out of range(UCS-2)!";
    if(_char_mapping[src])
        rs = 1;
    _char_mapping[src] = dest | (tag<<16);
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
    CHECK_LT(src_begin, MAX_UNICODE_CODEPOINT) << "src(being) out of range(UCS-2)!";
    CHECK_LT(dest_begin, MAX_UNICODE_CODEPOINT) << "dest(being) out of range(UCS-2)!";
    CHECK_LT(src_end, MAX_UNICODE_CODEPOINT) << "src(end) out of range(UCS-2)!";
    CHECK_LT(dest_end, MAX_UNICODE_CODEPOINT) << "dest(end) out of range(UCS-2)!";
    CHECK_LT(src_begin, src_end) << "start should less than end!";
    CHECK_LT(dest_begin, dest_end) << "start should less than end!";
    if ( (src_end - src_begin) != (dest_end - dest_begin) )
        return -1;

    for(u4 i = src_begin; i<=src_begin; i++) {
        if(_char_mapping[i])
            rs = 1;
        _char_mapping[i] = ( dest_begin + (i-src_begin) ) | (tag<<16);
    }
    return 0;
}

int
CharMapper::MappingPass(u4 src, u2 tag)
{
    int rs = 0;
    CHECK_LT(src, MAX_UNICODE_CODEPOINT) << "src out of range(UCS-2)!";

    if(_bDefaultPass & !tag)
        return 0; //do nothing
    if(_char_mapping[src])
        rs = 1;
    _char_mapping[src] = src | (tag<<16);
    return rs;
}

int
CharMapper::MappingRangePass(u4 src_begin, u4 src_end, u2 tag)
{
    int rs = 0;

    CHECK_LT(src_begin, MAX_UNICODE_CODEPOINT) << "src(being) out of range(UCS-2)!";
    CHECK_LT(src_end, MAX_UNICODE_CODEPOINT) << "src(end) out of range(UCS-2)!";
    CHECK_LT(src_begin, src_end) << "start should less than end!";

    if(_bDefaultPass & !tag)
        return 0; //do nothing

    for(u4 i = src_begin; i<=src_begin; i++) {
        if(_char_mapping[i])
            rs = 1;
        _char_mapping[i] = i | (tag<<16);
    }
    return 0;
}

u4
CharMapper::TransformScript(u4 src, u2* out_tag)
{
    return Transform(src, out_tag);
}

inline u4
CharMapper::Transform(u4 src, u2* out_tag)
{
    CHECK_LT(src, MAX_UNICODE_CODEPOINT) << "src out of range(UCS-2)!";
    if(_char_mapping[src]) {
        if(out_tag)
            *out_tag = _char_mapping[src] >> 16;
        return _char_mapping[src] & 0xFFFF;
    }
    // dictionary based lookup return 0.
    if(_bDefaultPass)
        return src;
    else
        return 0;
}

////////////////////////////////////////////////////////////////////////////////
///  Base & Basic Dictionary.
////////////////////////////////////////////////////////////////////////////////
BaseDict::BaseDict()
{
}

BaseDict::~BaseDict()
{
}

////////////////////////////////////////////////////////////////////////////////
int
BaseDict::Open(const char* dict_path, char mode)
{
    return 0;
}

int
BaseDict::Save(const char* dict_path){
    return 0;
}

int
BaseDict::Build()
{
    return 0;
}

int
BaseDict::Init(const LemmaPropertyDefine* props, int prop_count)
{
    return 0;
}

int
BaseDict::InitString(const char* prop_define, int str_define_len)
{
    return 0;
}

int
BaseDict::Insert(const char* term, int freq, const u4* pos, int pos_count)
{
    return 0;
}

int
BaseDict::SetProp(const char* term, const char* key, const void* data, int data_len)
{
    return 0;
}

int
BaseDict::GetProp(const char* term, const char* key, void** data, int* data_len)
{
    return 0;
}

int
BaseDict::Properties(const char* term, LemmaPropertyEntry** entries){
    return 0;
}

////////////////////////////////////////////////////////////////////////////////

} // end namespace mm

/* -- end of file -- */
