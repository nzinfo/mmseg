#include "basedict.h"


////////////////////////////////////////////////////////////////////////////////
/// CharMapper: Map Unicode Char -> Unicode Char in search form.
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
CharMapper::Mapping(u4 src, u4 dest, u1 tag)
{
    return 0;
}

int
CharMapper::MappingRange(u4 src_begin, u4 src_end, u4 dest_begin, u4 dest_end, u1 tag)
{
    return 0;
}

int
CharMapper::MappingPass(u4 src_begin, u1 tag)
{
    return 0;
}

int
CharMapper::MappingRangePass(u4 src_begin, u4 src_end, u1 tag)
{
    return 0;
}

inline u4
CharMapper::Transform(u4 src, u1* out_tag)
{
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

/* -- end of file -- */
