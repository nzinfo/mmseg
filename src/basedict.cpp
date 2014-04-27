#include <glog/logging.h>

#include "basedict.h"

#undef HAVE_MMAP

#include "csr_mmap.h"

namespace mm {

#define SafeDelete(_arg)		{ if ( _arg ) delete ( _arg );		(_arg) = NULL; }
#define SafeDeleteArray(_arg)	{ if ( _arg ) delete [] ( _arg );	(_arg) = NULL; }


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

    LOG(INFO) << "load charmap" << filename;
    //printf("I got %s %d.\n", filename, this->_bDefaultPass);
    // legacy code, use load into memory switch only. bLoadMem = true
    const u1* ptr = NULL;
    csr_mmap_t * dict_file = csr_mmap_file(filename, true);
    if(!dict_file) {
        rs = -404; goto CHARMAP_LOADFAIL; }
    ptr = (const u1*)csr_mmap_map(dict_file);
    // deal header.
    {
        mmseg_char_map_file_header* _header = (mmseg_char_map_file_header*)ptr;
        if(strncmp(charmap_head_mgc, _header->mg, 4) != 0) {
            rs = -415; goto CHARMAP_LOADFAIL; }
        // check length
        if(csr_mmap_size(dict_file) < sizeof(mmseg_char_map_file_header) + sizeof(_char_mapping)) {
            rs = -416; goto CHARMAP_LOADFAIL; }
        // do load , skip version now.
        this->_bDefaultPass = _header->flags & MMSEG_CHARMAP_FLAG_DEFAULT_PASS;
    }
    ptr += sizeof(mmseg_char_map_file_header);
    memcpy(_char_mapping, ptr, sizeof(_char_mapping));
    LOG(INFO) << "load charmap done " << filename ;

CHARMAP_LOADFAIL:
    {
        if(dict_file)
            csr_munmap_file(dict_file);
    }
    return rs;
}

int
CharMapper::Save(const char* filename)
{
    /*
     * Save to disk , defined in { file_header, char[] }
     */
    mmseg_char_map_file_header header;
    header.flags = MMSEG_CHARMAP_FLAG_DEFAULT_PASS;
    header.version = 1;
    memcpy(header.mg, charmap_head_mgc, sizeof(header.mg));
    {
        //write header
        std::FILE *fp  = std::fopen(filename, "wb");
        if(!fp)
            return -503;
        std::fwrite(&header,sizeof(mmseg_char_map_file_header),1,fp);
        std::fwrite(&_char_mapping,sizeof(_char_mapping),1,fp);
        std::fclose(fp);
    }
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
///  Base & Basic Dictionary. (Private)
////////////////////////////////////////////////////////////////////////////////
class BaseDictPrivate
{
public:
    // where store items
    // where store attributes?
    // where store string pool (hash)

public:
    //Darts::DoubleArray _dict;
};

////////////////////////////////////////////////////////////////////////////////
/// BaseDict
////////////////////////////////////////////////////////////////////////////////////

BaseDict::BaseDict()
    :_record_row_size(0)
{
    _p = new BaseDictPrivate();
}

BaseDict::~BaseDict()
{
    SafeDelete(_p);
}

////////////////////////////////////////////////////////////////////////////////
int
BaseDict::Open(const char* dict_path, char mode)
{

    return 0;
}

int
BaseDict::Save(const char* dict_path){
    /*
     *  Dictionary File Format:
     *  [header]
     *    - magic
     *    - version
     *    - flag ?
     *    - dictionary id eg. com.coreseek.mmseg.basic, sys should provide a feature load dict by id.
     *    - schema: how properties stored
     *      * pack: row_size, (column -> row ), if len(string) <= 4byte encode in pack.
     *    - dart size
     *    - prop count
     *    - data pool size
     *
     *  [darts]
     *    - dart array.
     *
     *  [properties]
     *    - fix size of record , packed
     *
     *  [datapool]
     *    - reuse pool ?
     *      (resue if entry's property is the same)
     *    - data pool
     *      (length, data)
     *
     * 在取结果时,可以选择要去的字段.
     */
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
    /*
     * 1 if no prop_defined ( input NULL | "")
     *      - add define "id":u4
     * 2 else
     *      - if no id in prop_define, append it.
     */
    char schema_define[1024];
    char *delim = ":;";
    char *tok = NULL;
    if(str_define_len > 1023)
        return -413; // schema define too large.

    memcpy(schema_define, prop_define, str_define_len);
    schema_define[str_define_len] = 0;

    do {
        tok = strtok(schema_define, delim);
        if(tok)
            printf("tok=%s\t", tok);
    }while(tok);

    return 0;
}

int
BaseDict::Insert(const char* term, unsigned int term_id, int freq, const u4* pos, int pos_count)
{
    return 0;
}

int
BaseDict::SetProp(unsigned int term_id,  const char* key, const void* data, int data_len)
{
    return 0;
}

int
BaseDict::GetProp(unsigned int term_id, const char* key, void** data, int* data_len)
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
