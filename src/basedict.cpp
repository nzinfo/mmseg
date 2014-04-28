#include <glog/logging.h>

#include <unordered_map>
#if defined __GNUC__ || defined __APPLE__
using std::unordered_map;
#else
using namespace std::tr1;
#endif

#include "darts.h"

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

class LemmaPropertyEntry
{
public:
    std::string prop_name;
    char  prop_type;
    int   row_offset;
    void* data;
    u4    data_len;
};

class LemmaEntry
{
public:
    unsigned int term_id;
    std::string term;
    int freq;
    std::vector<u4> pos;
    std::vector<LemmaPropertyEntry> props;
};

class BaseDictPrivate
{
public:
    // where store items
    // where store attributes?
    // where store string pool (hash)

    // Column Related
    int AddColumn(const std::string& def, int offset){
        column_offset[def] = offset;
        return 0;
    }

    void ResetColumn() {
        column_offset.clear();
    }

    char GetColumnType(const char* column_name) {
        // FIXME: use a hash ?
        for(unordered_map<std::string, int>::iterator it = column_offset.begin(); it !=  column_offset.end(); ++it) {
            if(strncmp(column_name, it->first.c_str()+2, strlen(column_name)) == 0)
                return it->first[0];
        }
        return ' ';  // type not found.
    }

    // Entry Related
    int AddEntry(LemmaEntry entry) {
        entries[entry.term_id] = entry;
        return 0;
    }

    int ResetEntry(bool bFreeEntryPropertyData = true) {
        /*
         * 1 free all entry's data_ptr
         * 2 reset hash
         */
        if(bFreeEntryPropertyData) {
            for(unordered_map<int, LemmaEntry>::iterator it = entries.begin(); it !=  entries.end(); ++it) {
                LemmaEntry & entry = it->second;
                for(std::vector<LemmaPropertyEntry>::iterator it = entry.props.begin(); it != entry.props.end(); ++it) {
                    //printf("tok=%s\t", it->key);
                    if(it->data) {
                        // FIXME: should alloc data in memory pool
                        free(it->data);
                        it->data = NULL;
                        it->data_len = 0;
                    }
                } // end for vector
            } // end for unordered_map
        } // end if bFreeEntryPropertyData
        entries.clear();
        return 0;
    }

public:
    Darts::DoubleArray _dict;
    unordered_map<int, LemmaEntry> entries;    // row uncompress format.

protected:
    unordered_map<std::string, int> column_offset;  // column-> offset in data block
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
BaseDict::Load(const char* dict_path, char mode)
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
BaseDict::SaveRaw(const char* dict_path)
{
    _p->_dict.save(dict_path);
    return 0;
}

int
BaseDict::LoadRaw(const char* dict_path)
{
    _p->_dict.open(dict_path);
    return 0;
}

int
BaseDict::Build()
{
    /*
     * Rebuild Darts from entries
     */
    //_p
    return 0;
}

int
BaseDict::Reset()
{
    // clear entry -> move to private ?
    _p->ResetEntry(true);// remove all entry.
    return 0;
}

int
BaseDict::Init(const LemmaPropertyDefine* props, int prop_count)
{
    /*
     * Calc & Build PropData Define.
     *  - combine u2 together.
     *  crc32(key): offset_in_data_row  u2 first; add together
     */
    int offset = 0;
    char buf[MAX_LEMMA_PROPERTY_NAME_LENGTH+2];
    const LemmaPropertyDefine* props_ptr = props;
    for(int i=0; i<prop_count; i++) {
        if(props_ptr[i].prop_type == '2') {
            snprintf(buf, sizeof(buf), "%c:%s", props_ptr[i].prop_type, props_ptr[i].key);
            _p->AddColumn(buf, offset);
            offset += 2;
        }
    }
    // check offset
    if(offset % 4 != 0) {
        offset += 2;
        LOG(INFO) << " inc offset -> " << offset;
    }
    for(int i=0; i<prop_count; i++) {
        if(props_ptr[i].prop_type == '2')
            continue;
        // do other property
        snprintf(buf, sizeof(buf), "%c:%s", props_ptr[i].prop_type, props_ptr[i].key);
        _p->AddColumn(buf, offset);
        // inc offset
        if(props_ptr[i].prop_type == '4' || props_ptr[i].prop_type == 's')
            offset += 4;
        if(props_ptr[i].prop_type == '8' )
            offset += 8;
    }

    _record_row_size = offset; // set row size.
    return 0;
}

int
BaseDict::InitString(const char* prop_define, int str_define_len)
{
    /*
     * assume prop_define in right format.
     * 1 if no prop_defined ( input NULL | "")
     *      - add define "id":u4
     * 2 else
     *      - if no id in prop_define, append it.
     */
    LemmaPropertyDefine prop;
    std::vector<LemmaPropertyDefine> props;
    char schema_define[1024];
    char delim[] = ":;";
    char *tok = schema_define;
    int sno = 0;
    size_t key_len = 0;

    if(str_define_len > 1023)
        return -413; // schema define too large.

    memcpy(schema_define, prop_define, str_define_len);
    schema_define[str_define_len] = 0;

    tok = strtok(schema_define, delim);
    sno++;
    while(tok) {
        if(sno % 2 == 1) {
            key_len = strlen(tok);
            CHECK_LT(key_len, MAX_LEMMA_PROPERTY_NAME_LENGTH) << "property define too long!";
            memcpy(prop.key, tok, key_len);
            prop.key[key_len] = 0;
        }else{
            // check type  2 4 8 s
            if(tok[0] == '2' || tok[0] == '4' || tok[0] == '8' || tok[0] == 's') {
                prop.prop_type = tok[0];
            } else {
                CHECK(false) << "property type invalid";
            }
            props.push_back(prop);
        }
        tok = strtok(NULL, delim);
        sno++;
    }; //end while true.

    if(0)
    {
        for(std::vector<LemmaPropertyDefine>::iterator it = props.begin(); it != props.end(); ++it) {
            printf("tok=%s\t", it->key);
        }
    }
    return Init(props.data(), props.size());
}

int
BaseDict::Insert(const char* term, unsigned int term_id, int freq, const u4* pos, int pos_count)
{
    // build entry.
    LemmaEntry entry;
    entry.term = term;
    entry.freq = freq;
    entry.term_id = term_id;
    for(int i=0; i<pos_count;i++)
        entry.pos.push_back(pos[i]);

    //_p->entries[entry.term_id] = entry;
    _p->AddEntry(entry); //copy construct
    return 0;
}

int
BaseDict::SetProp(unsigned int term_id,  const char* key, const char* data, int data_len)
{
    /*
     * How to alloc entry property's data ?
     *  - simple alloc data in this function. and free ?
     *  ! should use memory pool, all entry data alloc from pool
     *  ! as this code will be used only in diction pre-build, so...
     *
     *  ! should check term_id in hash ?
     */
    if( _p->entries.find(term_id) == _p->entries.end() )
        return -1;
    LemmaPropertyEntry prop_entry;
    prop_entry.prop_name = key;
    {
        // lookup property key
        prop_entry.prop_type = _p->GetColumnType(key);
        if(prop_entry.prop_type == ' ')
            return -2; // prop not found.
    }
    prop_entry.data = malloc(data_len);         //FIXME: should alloc from a memory pool
    memcpy(prop_entry.data, data, data_len);
    prop_entry.data_len = data_len;
    _p->entries.find(term_id)->second.props.push_back(prop_entry);
    return 0;
}

int
BaseDict::GetProp(unsigned int term_id, const char* key, char* data, int* data_len)
{
    // WARNING: if data_len extactly eq sizeof(it->data), will pass the function
    // if u wanna use string, should manually append \0 at caller side.
    if( _p->entries.find(term_id) == _p->entries.end() )
        return -1;

    std::string prop_name = key;
    std::vector<LemmaPropertyEntry>& props = _p->entries.find(term_id)->second.props;
    for(std::vector<LemmaPropertyEntry>::iterator it = props.begin(); it != props.end(); ++it) {
        //printf("tok=%s\t", it->prop_name.c_str() );
        if( it->prop_name == prop_name ) {
            if(*data_len >= it->data_len) {
                memcpy(data, it->data, it->data_len);
                *data_len = it->data_len;
                return 0;   // done. ok.
            } else {
                *data_len = it->data_len;
                return -2;  // ? data buffer too small
            }
        }
    } //end for
    return -3; //? key not found
}

int
BaseDict::SetPropInteger(unsigned int term_id, const char* key, u8 v)
{
    // have to do alloc, the reset does not care about type.
    char* ptr = (char*) &v;
    return SetProp(term_id, key, ptr, 8);
}

int
BaseDict::GetPropInteger(unsigned int term_id, const char* key, u8* v)
{
    char buf[8];
    int  buf_size = 8;
    int rs = GetProp(term_id, key, buf, &buf_size);
    if(rs == 0)
        *v = * ( (u8*)buf );
    return rs;
}

/*
int
BaseDict::Properties(const char* term, LemmaPropertyEntry** entries){
    return 0;
}
*/

////////////////////////////////////////////////////////////////////////////////

} // end namespace mm

/* -- end of file -- */
