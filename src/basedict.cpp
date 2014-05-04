#include <glog/logging.h>

/*
#if defined __GNUC__ || defined __APPLE__
using std::unordered_map;
#else
using namespace std::tr1;
#endif
*/

#include "darts.h"

#include "basedict.h"

#undef HAVE_MMAP
#include "csr_mmap.h"

#include "csr_utils.h"

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
const char basedict_head_mgc[] = "TERM";

typedef struct _mmseg_char_map_file_header{
    char mg[4];
    short version;
    short flags;
    u4    page_mask;    // 使用了哪几个平面
}mmseg_char_map_file_header;

typedef struct _mmseg_dict_file_header{
    char mg[4];
    short version;
    short flags;
    char dictname[128];
    u4   dict_rev;              // dictionay reversion.
    u8   timestamp;             // create_at ?
    u4   item_count;            // 多少词条
    u4   entry_data_offset;     // 词条属性的偏移量, 从文件头开始计算, 包括 head. idx_data_len
}mmseg_dict_file_header;

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
    const u1* ptr = NULL;
    csr_mmap_t * dict_file = csr_mmap_file(filename, true);
    if(!dict_file) {
        rs = -404; goto CHARMAP_LOADFAIL; }
    ptr = (const u1*)csr_mmap_map(dict_file);
    // deal header
    {
        _header = (mmseg_char_map_file_header*)ptr;
        if(strncmp(charmap_head_mgc, _header->mg, 4) != 0) {
            rs = -415; goto CHARMAP_LOADFAIL; }
        // check length
        if(csr_mmap_size(dict_file) < sizeof(mmseg_char_map_file_header) + sizeof(_char_mapping_base)) {
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

////////////////////////////////////////////////////////////////////////////////
///  Base & Basic Dictionary. (Private)
////////////////////////////////////////////////////////////////////////////////

class LemmaPropertyEntry
{
public:
    std::string prop_name;
    char  prop_type;
    int   row_offset;
    void* data;     // if prop_type = 2 4; data is real value, do not use high 32bit on 64bit machine, for 32-bit compt
    u2    data_len; // if type_type = 8, this is high 4byte. void* point to a memory addr only when type = s
};

class LemmaEntry
{
public:
    unsigned int term_id;
    std::string term;
    std::vector<LemmaPropertyEntry> props;
};

bool EntryAscOrderCmp (const LemmaEntry& elem1, const LemmaEntry& elem2 )
{
    return elem1.term < elem2.term;
}

/*
 * FIXME: should change darts.h make this type keyset work
 */
template <typename T>
class Keyset : public Darts::Details::Keyset<T>{
 public:
  Keyset(std::size_t num_keys, const LemmaEntry* entries) :
      Darts::Details::Keyset<int>(num_keys, NULL, NULL, NULL),
      num_keys_(num_keys), entries_(entries)
  {
    //printf("111111111111\n");
  }

  std::size_t num_keys() const {
    //printf("111111111111====\n");
    return num_keys_;
  }
  const Darts::Details::char_type *keys(std::size_t id) const {
    //printf("111111111111kkkkkkkkkk====\n");
    return entries_[id].term.c_str();
  }
  Darts::Details::uchar_type keys(std::size_t key_id, std::size_t char_id) const {
    if (has_lengths() && char_id >= entries_[key_id].term.length())
      return '\0';
    return entries_[key_id].term[char_id];
  }

  bool has_lengths() const {
    return true;
  }
  std::size_t lengths(std::size_t id) const {
    //if (has_lengths())
    {
      return entries_[id].term.length();
    }
    /*
    std::size_t length = 0;
    while (keys_[id][length] != '\0') {
      ++length;
    }
    return length;
    */
  }

  bool has_values() const {
    return true;
  }

  const Darts::Details::value_type values(std::size_t id) const {
    //if (has_values())
    {
      return static_cast<Darts::Details::value_type>(entries_[id].term_id);
    }
    //return static_cast<value_type>(id);
  }

 private:
  std::size_t num_keys_;
  const LemmaEntry* entries_;

  // Disallows copy and assignment.
  Keyset(const Keyset &);
  Keyset &operator=(const Keyset &);
};


////////////////////////////////////////////////////////////////////////////////
/// BaseDictSchema
////////////////////////////////////////////////////////////////////////////////////
int
BaseDictSchema::AddColumn(const std::string& def, u2 idx){
    //LOG(INFO) << "new column append " << def << " @"<<idx;
    column_by_idx[idx] = def;                          // full_name  -> offset in datapkg.
    column_type_and_idx[def.c_str()+2] = def[0] | (idx<<8);    // raw_name: type
    CHECK_LT(column_by_idx.size(), MAX_PROPERTY_COUNT) << "max column reached!";
    return 0;
}

inline char
BaseDictSchema::GetColumnType(int idx)
{
    return column_by_idx[idx][0]; // 4:id, s:thes
}

const std::string
BaseDictSchema::GetSchemaDefine() {
    std::stringstream ss;
    //schema = [total_schema_len][type(2b):offset(2b):name],pos is id
    for(short i=0; i<MAX_PROPERTY_COUNT; i++) {
        unordered_map<int, std::string>::iterator it = column_by_idx.find(i);
        if(it != column_by_idx.end())
            ss << it->second << ";" ;
    }
    return ss.str();
}

int
BaseDictSchema::InitString(const char* prop_define, int str_define_len)
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
            // check type  2 4 8 s
            if(tok[0] == '2' || tok[0] == '4' || tok[0] == '8' || tok[0] == 's') {
                prop.prop_type = tok[0];
            } else {
                CHECK(false) << "property type invalid";
            }
        }else{
            key_len = strlen(tok);
            CHECK_LT(key_len, MAX_LEMMA_PROPERTY_NAME_LENGTH) << "property define too long!";
            memcpy(prop.key, tok, key_len);
            prop.key[key_len] = 0;

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

////////////////////////////////////////////////////////////////////////////////
/// BaseDictPrivate
////////////////////////////////////////////////////////////////////////////////////

class BaseDictPrivate
{
public:
    BaseDictPrivate(BaseDictSchema& schema):
        entry_data_(NULL)
      , entry_key_string_pool_(NULL)
      , entry_keys_(NULL)
      , entry_values_(NULL)
      , _schema(schema)
    {

    }

    ~BaseDictPrivate() {
        if(entry_data_) free(entry_data_);
        if(entry_key_string_pool_) free(entry_key_string_pool_);
        if(entry_keys_) free(entry_keys_);
        if(entry_values_) free(entry_values_);
    }

    // Entry Related
    int AddEntry(LemmaEntry& entry) {
        if(entry_data_)
            return -405; //read only dict.
        //FIXME: change here if use entry's string pool
        LemmaEntry& origin_entry = entries[entry.term_id];
        if(origin_entry.term_id) {
            //  free data
            for(std::vector<LemmaPropertyEntry>::iterator it = entry.props.begin(); it != entry.props.end(); ++it) {
                if(it->data && it->prop_type == 's') {
                    free(it->data);
                    it->data = NULL;
                    it->data_len = 0;
                }
            } // end for vector
            LOG(INFO) << "term id " << origin_entry.term_id << " " << origin_entry.term << " replaced by " << entry.term;
        }
        entries[entry.term_id] = entry;
        return 0;
    }

    int ResetEntry(bool bFreeEntryPropertyData = true) {
        if(entry_data_)
            return -405; //read only dict.
        /*
         * 1 free all entry's data_ptr
         * 2 reset hash
         */
        if(bFreeEntryPropertyData) {
            for(unordered_map<int, LemmaEntry>::iterator it = entries.begin(); it !=  entries.end(); ++it) {
                LemmaEntry & entry = it->second;
                for(std::vector<LemmaPropertyEntry>::iterator it = entry.props.begin(); it != entry.props.end(); ++it) {
                    if(it->data && it->prop_type == 's') {
                        // FIXME: should alloc data in memory pool
                        free(it->data);
                        it->data = NULL;
                        it->data_len = 0;
                    }
                } // end for vector
            } // end for unordered_map
        } // end if bFreeEntryPropertyData
        entries.clear();
        ResetDart();
        return 0;
    }

    // Darts Related
    int BuildDart(unsigned int *dup_term_buf, int buf_length) {
        /*
         *  1 check have dup term
         *  2 sort key in asc order
         *  3 clear darts
         *  4 call darts build
         *
         *  FIXME: preformace is NOT a consider.
         *  仅用于内存中创建, 实际代码中, 构造Darts 在词库的 Load 阶段
         */
        std::vector<LemmaEntry> v;
        v.reserve(entries.size());

        {
            // check dup
            unsigned int* dup_term_buf_ptr = dup_term_buf;
            unsigned int* dup_term_buf_end = dup_term_buf + buf_length;
            * dup_term_buf_ptr = 0;

            unordered_map<std::string, int> keys_hash;
            for(unordered_map<int, LemmaEntry>::iterator it = entries.begin();
                    it !=  entries.end(); ++it) {
                LemmaEntry & entry = it->second;
                if ( keys_hash.find(entry.term) != keys_hash.end() ) {
                    if(dup_term_buf_ptr < dup_term_buf_end) {
                        *dup_term_buf_ptr = entry.term_id;
                        dup_term_buf_ptr ++;
                    }
                } // end if find
                keys_hash[entry.term] = entry.term_id;
                v.push_back(entry);
            } // for

            if (*dup_term_buf != 0)
                return -1; //dup entry found.
        }

        {
            // sort in alphabet order
            std::sort(v.begin(), v.end(), EntryAscOrderCmp);

            /*
            //FIXME: do NOT remove the code, can be fixed by doing a bit darts hacking.
            Darts::Details::DoubleArrayBuilder builder(NULL);
            Keyset<int> keyset(entries.size(), v.data());
            builder.build(keyset);

            std::size_t size = 0;
            Darts::Details::DoubleArrayUnit *buf = NULL;
            builder.copy(&size, &buf);

            // data data.
            _dict.set_array(buf, size);
            */
            // slow darts build, but it works.
            std::vector <Darts::DoubleArray::key_type *> key;
            std::vector <Darts::DoubleArray::value_type> value;
            for(std::vector<LemmaEntry>::iterator it = v.begin(); it != v.end(); ++it) {
                char* ptr = &( it->term[0] );
                key.push_back(ptr);
                value.push_back(it->term_id);
            }
            _dict.build(key.size(), &key[0], 0, &value[0] ) ;
        }

        return 0;
    }

    int ResetDart() {
        _dict.clear();
        return 0;
    }

    // Save & Load
    int Load(const char* filename, bool keep_term_data) {
        LOG(INFO) << "load dictionary begin, " << filename;

        u4 total_read_count = 0;
        std::FILE *fp = std::fopen(filename, "rb");
        //FIXME: should check ptr's length.
        if (!fp) return -1;

        u4 file_size = 0;
        {
            if (std::fseek(fp, 0L,     SEEK_END) != 0) return -1;
            file_size = std::ftell(fp);
            if (std::fseek(fp, (long)0, SEEK_SET) != 0) return -1;
        }
        //process header
        total_read_count += sizeof(mmseg_dict_file_header);
        if (1 == std::fread(&header, sizeof(mmseg_dict_file_header), 1, fp))
        {
            //Check file magic header
            if(strncmp(header.mg, basedict_head_mgc, 4) != 0)
                return -2; //file type error.
            LOG(INFO) << "dict name " << header.dictname;
            LOG(INFO) << "entry count " << header.item_count;
            LOG(INFO) << "dict rev " << header.dict_rev;
            LOG(INFO) << "dict entry_data_offset " << header.entry_data_offset;
            dict_name_.assign(header.dictname);

        } else
            return -1;

        //schema
        u4 schema_str_length = 0;
        total_read_count += sizeof(u4);
        if (1 == std::fread(&schema_str_length, sizeof(u4), 1, fp))
        {
            _schema.ResetColumn(); //clear
            char* schema_buf = (char*)malloc(schema_str_length+1);
            schema_buf[schema_str_length] = 0;
            total_read_count += schema_str_length;
            if (schema_str_length != std::fread(schema_buf, 1, schema_str_length, fp))
                return -1;
            LOG(INFO) << "dict schema " <<schema_buf;
            _schema.InitString(schema_buf, schema_str_length);
            LOG(INFO) << "dict schema (rebuild)" << _schema.GetSchemaDefine();
            free(schema_buf);
        } else
            return -1;

        //item's values  //must hava a void* to char* [], else cause a compiler type detect error.
        void* string_buf_ptr = malloc(sizeof(u1ptr) * header.item_count); // addtional one space for get char*[] easy.
        u1ptr* string_begin = (u1ptr*)string_buf_ptr;

        u4* values_ptr = (u4*)malloc(sizeof(u4) * header.item_count);
        std::size_t* string_length_ptr = (std::size_t*)malloc(sizeof(std::size_t) * header.item_count); //the length of the strings.
        total_read_count += sizeof(u4) * header.item_count;
        if (header.item_count == std::fread(values_ptr, sizeof(u4), header.item_count, fp))
        {
            //  check value's offset.
            if(0) {
                for(int i=0; i< header.item_count; i++) {
                    printf("item value =%d\n", values_ptr[i]);
                }
            }
        }else
            return -1;

        //item's strings.
        u4 string_pool_begin_pos = sizeof(mmseg_dict_file_header) + sizeof(u4) + schema_str_length + sizeof(u4) * header.item_count;
        u4 string_pool_size =  header.entry_data_offset - string_pool_begin_pos;
        u1* string_pool_ptr = (u1*)malloc(sizeof(u1) * string_pool_size);
        LOG(INFO) << "term string pool offset is (read at) " <<  total_read_count;
        total_read_count += sizeof(u1) * string_pool_size;
        if (string_pool_size == std::fread(string_pool_ptr, sizeof(u1), string_pool_size, fp))
        {
            u1* s_ptr = string_pool_ptr;
            for(int i = 0; i < header.item_count; i++) {
                string_length_ptr[i] = strlen(( char*)s_ptr);
                string_begin[i] = s_ptr;
                CHECK_NE(string_length_ptr[i], 0) << "string length cannot be 0. @"<<i;
                s_ptr += string_length_ptr[i] + 1;
            } // end for.
        }else
            return -1;
        // load data
        u4 entry_data_length = file_size-header.entry_data_offset;
        LOG(INFO) << "load dictionary total entry_data length " << entry_data_length << " index block is " << header.entry_data_offset << " total read:"<<total_read_count;

        u1* entry_data = (u1*)malloc(entry_data_length);
        if (entry_data_length == std::fread(entry_data, sizeof(u1), entry_data_length, fp))
        {
            // do nothing ?
        }else
            return -1;

        // check string
        if ( 0 ) {
            for(int i=0; i< header.item_count; i++) {
                printf("item=%s\n", string_begin[i]);
            }
        }
        //build darts?
        LOG(INFO) << "load dictionary finished, " << filename;
        {
            if(fp)
                std::fclose(fp);
        }
        // free all?
        this->entry_data_ = entry_data;
        this->entry_data_end_ = entry_data + entry_data_length;
        if(!keep_term_data)
        {
            LOG(INFO) << "build darts index, " << filename;
            // try build darts
            _dict.clear();
            const char* const * keys_ptr = (char* const *)string_begin;
            Darts::DoubleArray::value_type* darts_values_ = (Darts::DoubleArray::value_type*)values_ptr;
            if(0) {
                for(int i=0; i< header.item_count; i++) {
                    printf("key=%s, id=%d\n", keys_ptr[i], darts_values_[i]);
                }
            }

            _dict.build(header.item_count, keys_ptr, string_length_ptr, darts_values_ ) ;
            LOG(INFO) << "dictionary darts index built, " << filename;
            free(values_ptr);
            free(string_pool_ptr);
            free(string_buf_ptr);
        }else{
            this->entry_key_string_pool_ = string_pool_ptr;
            this->entry_values_ = values_ptr;
            this->entry_keys_ = string_begin;
        }
        free(string_length_ptr); // free string length buffer. will recalc when dict merge.

        return 0;
    }

    int Save(const char* filename, int dict_rev) {
        /*
         *  In Save & Load, I do NOT care about dup term.
         *  Raw File Format:
         *  [ hearder][schema][string_len_index][value_index][string_index]
         *               {the offset of raw value}  -> values { id, attrs }
         *  raw_data = data_len, fixdata, strings,  opt for easy transfer via network.
         *  schema = [total_schema_len][type(2b):offset(2b):name],pos is id
         *  FIXME: compress file?
         */
        LOG(INFO) << "save dictionary begin " << filename;
        std::vector<LemmaEntry> v;
        v.reserve(entries.size());
        {
            size_t total_string_len = 0;
            for(unordered_map<int, LemmaEntry>::iterator it = entries.begin();
                    it !=  entries.end(); ++it) {
                LemmaEntry & entry = it->second;
                v.push_back(entry);
                CHECK_EQ( entry.term.length(), strlen(entry.term.c_str())) << "string have \\0. " << entry.term << " term length " << entry.term.length();
                CHECK_NE( entry.term.length(), 0) << "string length cannot be 0. ";
                total_string_len += entry.term.length() + 1;    // will append a \0
            } // for
            LOG(INFO) << "total entry count " <<  v.size();
            // sort in alphabet order
            std::sort(v.begin(), v.end(), EntryAscOrderCmp);
            LOG(INFO) << "sort entry done ";
            // try build dump data.
            // header + size(u4){property data}* term_count + total_string_len + (zero)*term_count ,  不额外保存字符串长度信息，而改为0， 因为多数字符串长度小于128
            //  + [ entry_data ]
            {
                u4 schema_define_len = 0;   // how to define schema in file
                const std::string& schema_str = _schema.GetSchemaDefine();
                schema_define_len = sizeof(u4) + schema_str.size(); // length:string_data
                LOG(INFO) << "schema define " << schema_str ;
                u4 idx_data_len = sizeof(mmseg_dict_file_header) + schema_define_len +
                        sizeof(u4) * v.size() +  total_string_len;

                u1 * idx_data = (u1*)malloc( idx_data_len );
                // pointers
                mmseg_dict_file_header* header = (mmseg_dict_file_header*) idx_data;
                // 词条属性偏移量的列表, 对应 darts 的 value (定长)
                u4* term_prop_idx_begin = (u4*)( idx_data + sizeof(mmseg_dict_file_header) + schema_define_len );
                u4* term_prop_idx_ptr = term_prop_idx_begin;
                // 词条值的字符串池, \0 标记为结尾
                u1* term_pool_begin = idx_data + sizeof(mmseg_dict_file_header) + schema_define_len + sizeof(u4) * v.size();
                u1* term_pool_ptr = term_pool_begin;

                std::FILE *fp  = std::fopen(filename, "wb");
                if(!fp)
                    return -503;

                std::fwrite(idx_data, sizeof(u1), idx_data_len, fp);
                // Simple & stupid code, assume no entry have more than 128K property.
                void* entry_data = malloc(MAX_ENTRY_DATA);
                int rs = 0;
                int entry_data_length = 0;
                int entry_data_total_length = 0;
                //
                //*term_prop_idx_ptr = entry_data_total_length;
                LOG(INFO) << "term string pool offset is " <<  term_pool_ptr - idx_data;
                for(std::vector<LemmaEntry>::iterator it = v.begin(); it != v.end(); it++) {
                    // 设置字符串实际值的 string-pool
                    memcpy(term_pool_ptr, it->term.c_str(), it->term.length());
                    term_pool_ptr += it->term.size();
                    *term_pool_ptr = 0;
                    term_pool_ptr++;  // append \0

                    entry_data_length = 0;
                    LemmaEntry& entry = *it;
                    rs = GetEntryData(entry, entry_data, &entry_data_length);
                    if(rs == 0 && entry_data_length) {
                        // inc prop offset.
                        entry_data_total_length += entry_data_length;
                        *term_prop_idx_ptr = entry_data_total_length - entry_data_length;
                        term_prop_idx_ptr ++;
                        // at the loop end, term_prop_idx_ptr will point to term_pool_begin
                        CHECK_LT((const void*)term_prop_idx_ptr, (const void*)(term_pool_begin+1)) << "overwrite to string pool!";
                        // write to...
                        std::fwrite(entry_data, sizeof(u1), entry_data_length, fp);
                    } //end if
                    if(0) {
                        printf("build key=%s, offset=%d .. %d\n", entry.term.c_str(), *(term_prop_idx_ptr), entry_data_length);
                        u4* ptr = (u4*)( idx_data + sizeof(mmseg_dict_file_header) + schema_define_len );
                        for(int i=0; i< v.size(); i++) {
                            printf("item value===== =%d\n", ptr[i]);
                        }
                    }
                } // end for
                // update the last ptr

                LOG(INFO) << "save dictionary total entry_data length " << entry_data_total_length << " index block is " << idx_data_len;
                // rewrite header.
                {
                    // header schema_length:u4; schema offset_list; string_pool, entry_data
                    memcpy(header->mg, basedict_head_mgc, 4);
                    header->version = 1;
                    header->flags = 0;
                    memcpy(header->dictname, dict_name_.c_str(), dict_name_.size());
                    header->dictname[dict_name_.size()] = 0;

                    header->dict_rev = dict_rev;
                    header->timestamp = currentTimeMillis();
                    header->item_count = v.size();
                    header->entry_data_offset = idx_data_len;

                    // build schema
                    u1* schema_ptr = idx_data + sizeof(mmseg_dict_file_header);
                    *(u4*)schema_ptr = schema_str.size();
                    memcpy(schema_ptr+sizeof(u4), schema_str.c_str(), schema_str.size());
                    // entry offset
                    std::fseek(fp, 0, SEEK_SET);
                    std::fwrite(idx_data, sizeof(u1), idx_data_len, fp);
                    //printf("idx=%d, entry=%d\n", idx_data_len, entry_data_total_length);
                    if(0) {
                        term_prop_idx_ptr = (u4*)( idx_data + sizeof(mmseg_dict_file_header) + schema_define_len );
                        for(int i=0; i< header->item_count; i++) {
                            printf("item value =%d\n", term_prop_idx_ptr[i]);
                        }
                    }
                }
                free(entry_data);
                free(idx_data);
                // close file
                std::fclose(fp);
            }
        }
        LOG(INFO) << "save dictionary done " << filename;
        return 0;
    }

    int GetEntryData(LemmaEntry& entry, void* data, int* data_size) {
        /*
         *  EntryData: 的格式
         *  ID 4B: Entry 对应外部关联的编号 最高位为 0
         *  FLAG 2B: 最高位永远为 1, 可选
         *   SIZE 2B 如果 存在 FLAG 则存在;
         *   DATA 整数的存储,如果是字符串存储 2B 的偏移量, 从 DATA 起始 开始计算
         *   STRING 包括的全部字符串信息, 使用 \0 标记结尾
         */
        short prop_idx = 0;
        u2  property_map_flags = 0;  // [1~15], 1 if the property is exist , 0 not.
        u2  idx2entry_prop[MAX_PROPERTY_COUNT] = {0};      // 按属性顺序的 idx -> prop_entry 的映射表
        for(std::vector<LemmaPropertyEntry>::iterator it = entry.props.begin(); it != entry.props.end(); ++it) {
            prop_idx = _schema.GetColumnIdx(it->prop_name.c_str());
            idx2entry_prop[prop_idx] = ( it - entry.props.begin() ) + 1;    // if 0 means, property not exist, so needs plus 1
            property_map_flags |= 1 << prop_idx;
        }
        *data_size = 0;
        // if no property data
        if(property_map_flags ==0 ) {
           * ( (u4*) data ) = entry.term_id; // should check data_size
           *data_size = 4;
           return 0;
        }

        u1 string_pool[MAX_ENTRY_DATA] = {0};
        u1 * data_ptr = (u1*)data;  // pointer to id, flag & size
        u1 * data_begin = data_ptr;
        data_ptr += 8;  // id:4; flag&size: 4
        u1 * string_pool_ptr = string_pool;
        for(short i=0; i<MAX_PROPERTY_COUNT; i++) {
            if(idx2entry_prop[i]) {
                LemmaPropertyEntry & prop_entry = entry.props[ idx2entry_prop[i] - 1];
                // A bit trick, I use prop_entry.data pointer save u2 u4 's real value.
                void* entry_data_ptr = &(prop_entry.data);
                switch (prop_entry.prop_type ) {
                case '2':{
                        u2* ptr = (u2*)data_ptr;
                        *ptr = *(u2*)entry_data_ptr;
                        data_ptr += 2;
                    }
                    break;
                case '4':{
                        u4* ptr = (u4*)data_ptr;
                        *ptr = *(u4*)entry_data_ptr;
                        data_ptr += 4;
                    }
                    break;
                case '8':{
                        u4* ptr = (u4*)data_ptr;
                        *ptr = *(u4*)entry_data_ptr;
                        ptr[1] = prop_entry.data_len;
                        data_ptr += 8;
                    }
                    break;
                case 's':{
                        // save a u2 offset here.
                        u2* ptr = (u2*)data_ptr;
                        *ptr = (u2) (string_pool_ptr - string_pool);
                        data_ptr += 2;
                        //Check buffer size.
                        CHECK_LT(string_pool_ptr - string_pool + prop_entry.data_len, MAX_ENTRY_DATA) << "entry string data too large, id=" << entry.term_id;
                        memcpy(string_pool_ptr, prop_entry.data, prop_entry.data_len);
                        //printf("orig data %s\n", prop_entry.data);
                        string_pool_ptr += prop_entry.data_len;
                        *string_pool_ptr = 0;
                    }
                    break;
                 default:
                    CHECK(false) << "invalid entry_prop_type." << prop_entry.prop_type;
                }
            }
        } //end for short
        // data_begin + 4: first 4 bytes; 1<<31 mark as a data-block; property_map_flags how may fields; (data_ptr - (data_begin + 8) ) real data size.
        * (u4*)(data_begin + 4)  = (1<<31) | ( property_map_flags << 16 ) | (data_ptr - (data_begin + 8) );
        *data_size = data_ptr - data_begin;
        //printf("string pool is %s\n", string_pool);
        if(string_pool_ptr!=string_pool)
            memcpy(data_ptr, string_pool, string_pool_ptr-string_pool);
        return 0;
    }


    const void* GetOnDiskEntryProperty(u4 value, int* data_size) {
        /*
         *  返回记录对应的数据区, 如果不存在, 返回 NULL
         */
        if(!entry_data_)
            return  NULL;
        CHECK_LT(value, entry_data_end_ - entry_data_) << "value[offset] access beyond data border.";  // return NULL ?
        // data_begin + 4: first 4 bytes; 1<<31 mark as a data-block; property_map_flags how may fields; (data_ptr - (data_begin + 8) ) real data size.
        u4* data_ptr = (u4*)(entry_data_ +value);
        u4  flag = data_ptr[1];
        // check 1st bitflag
        if(flag >> 31 & 0x1)
        {
            *data_size = 8 + flag & 0xFFFF;
            return data_ptr;
        }else{
            *data_size = 4;
            return data_ptr;
        }
    }

    i4 GetOnDiskEntryOffset(const void* data, u4 data_size, const char* key) {
        /*
         *  返回 属性 在 Term 的属性数据中的偏移量;
         *  如果错误 返回 负数
         */
        if(!entry_data_)
            return -100;

        if(entry_data_<=data && data < entry_data_end_)
        {
            // in right data range.
            u4* data_ptr = (u4*)data;
            u4  flag = data_ptr[1];
            u1  prop_flag =  (flag >> 16) & 0x7F;

            if(key == NULL || strcmp("id", key) == 0)
                return  0; // default is term id
            if(! (flag >> 31 & 0x1) )
                return -1;  // no such data, if flag not mark 1 at highest bit, it's next entry's termid.

            short prop_idx = _schema.GetColumnIdx(key);
            if(prop_idx < 0)
                return -2; // no such column

            short offset = 8;
            for(short i = 0; i <MAX_PROPERTY_COUNT; i++ ) {
                if(i == prop_idx) {
                    //printf("-----%d---%d\n", prop_flag, i);
                    if (! (prop_flag >> i & 0x1 ) )
                        return -3; // not such prop.
                }
                if ( prop_flag >> i & 0x1 ) {
                    // have prop
                    char prop_type = _schema.GetColumnType(i);
                    if(i == prop_idx)
                        break;
                    switch(prop_type) {
                    case '2': {
                        offset += 2;
                        break;
                        }
                    case '4': {
                        offset += 4;
                        break;
                        }
                    case '8': {
                        offset += 8;
                        break;
                        }
                    case 's':{
                        offset += 2;
                        break;
                        }
                    }
                }// end if
          } //  end for.
          CHECK_LT(offset, data_size) << "property offset beyond border.";
          return offset;
        }else
          return -200; // invalid data pointer.
    }

    u4 GetOnDiskEntryPropertyU4 (const void* data, u4 data_size, const char* key, u4 def_value){
        if(!entry_data_)
            return def_value;

        i4 offset = GetOnDiskEntryOffset(data, data_size, key);
        if(offset < 0)
            return def_value;

        u1* prop_ptr = (u1*)data;
        prop_ptr += offset; // FIXME: check offset .
        return *(u4*)prop_ptr;
    }


    const char* GetOnDiskEntryPropertyString(const void* data, u4 data_size, const char* key, int* data_len){
        if(!entry_data_)
            return NULL;

        i4 offset = GetOnDiskEntryOffset(data, data_size, key);
        if(offset < 0)
            return NULL;

        return NULL;
    };

public:
    Darts::DoubleArray _dict;
    mmseg_dict_file_header header;
    // in memory
    unordered_map<int, LemmaEntry> entries;    // row uncompress format.

    // from disk
    u1*         entry_data_;    // 词条的属性数据
    u1*         entry_data_end_;
    u1*         entry_key_string_pool_;  // 词条字符串数据的时间存储位置
    u1ptr*      entry_keys_;    // 对词条的索引, 用于构造 darts
    u4*         entry_values_;  // 词条实际属性的偏移量，在 entry_data_ 中

    // 通用属性
    std::string dict_name_;     //
    BaseDictSchema& _schema;
};

////////////////////////////////////////////////////////////////////////////////
class DictMatchResultPrivate
{
public:
    DictMatchResultPrivate() {
        Reset();
    }

    inline void Reset() {
        _reulst_idx = 0;
        memset(_results, 0, sizeof(_results));
    }

    Darts::DoubleArray::result_pair_type _results[MAX_PREFIX_SEARCH_RESULT];
    int _reulst_idx;
};

DictMatchResult::DictMatchResult()
{
    _p = new DictMatchResultPrivate();
}

DictMatchResult::~DictMatchResult()
{
    SafeDelete(_p);
}

u8
DictMatchResult::GetResult(){
    return GetResult(_p->_reulst_idx-1);
}

u8
DictMatchResult::GetResult(int idx)
{
    u8 v = 0;
    if(_p->_reulst_idx) {
        Darts::DoubleArray::result_pair_type & rs = _p->_results[idx];
        v = rs.value;
        v = (v << 32) | rs.length;
    }
    return v; //offset, length
    //return _p->_reulst_idx;
}

inline void*
DictMatchResult::GetResultPtr()
{
    return _p->_results;    // type: Darts::DoubleArray::result_pair_type
}

inline void
DictMatchResult::ClearResult()
{
    _p->Reset();
}

inline int
DictMatchResult::AddResult(u4 value, u4 length){
    Darts::DoubleArray::result_pair_type & rs = _p->_results[_p->_reulst_idx];
    rs.length = length;
    rs.value = value;
    _p->_reulst_idx ++;

    return _p->_reulst_idx;
}

////////////////////////////////////////////////////////////////////////////////
/// BaseDict
////////////////////////////////////////////////////////////////////////////////////

BaseDict::BaseDict()
{
    _p = new BaseDictPrivate(_schema);
}

BaseDict::~BaseDict()
{
    SafeDelete(_p);
}

////////////////////////////////////////////////////////////////////////////////
int
BaseDict::Load(const char* dict_path, char mode)
{
    // mode : r     raw     ; c combine with many dict.
    if(mode == 'c')
        return _p->Load(dict_path, true); // keep string keys
    if(mode == 'r')
        return _p->Load(dict_path, false); // build dart & free raw string.
    return -405;
}

int
BaseDict::Save(const char* dict_path, int dict_rev){

    /*
     *  Dictionary File Format:  废弃, 改为在词库加载时, 动态计算 darts, darts 可以被缓存为文件
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
     * Note:
     *  1 只保存词库的原始格式 header + properties + datapool, 不再保存 dart
     *  2 在分词法部分加载多个词库后, 可以构造统一的 dart, 也可以分词库构造 dart
     *  3 更新| Reload 词库后, 主 dart 对应的词库作废, 读取分词库的.
     *  4 优化过程为重新构造 新的 dart
     *  引入 zlib, 压缩存储?
     *
     *  Raw File Format:
     *  [ hearder][schema][string_len_index][value_index][string_index]  {the offset of raw value}  -> values { id, attrs }
     *  raw_data = data_len, fixdata, strings,  opt for easy transfer via network.
     *
     */
    return _p->Save(dict_path, dict_rev);
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
     * -
     */
    int n = 0;
    unsigned int dup_term_buf[100] = {0};
    n = _p->BuildDart(dup_term_buf, 100);
    if(n == -1) {
       unsigned int * ptr = dup_term_buf;
       while(*ptr) {
         LOG(INFO) << "term id dup." << *ptr;
         ptr++;
       }
    }
    return n;
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
    return _schema.Init(props, prop_count);
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
    return _schema.InitString(prop_define, str_define_len);
}

int
BaseDict::SetDictName(const char* dict_name)
{
    _p->dict_name_.assign(dict_name);
    return 0;
}

int
BaseDict::ExactMatch(const char* key, size_t key_len, DictMatchResult& match_rs)
{
    // return current result count.
    Darts::DoubleArray::result_pair_type  rs;
    _p->_dict.exactMatchSearch(key,rs, key_len);
    //printf("rs.pos = %d, rs.value=%d\n", rs.length, rs.value);
    return match_rs.AddResult(rs.length, rs.value);
}

int
BaseDict::PrefixMatch(const char* buf, size_t key_len, DictMatchResult& rs)
{
    Darts::DoubleArray::result_pair_type* result = ( Darts::DoubleArray::result_pair_type* )rs.GetResultPtr();
    int num =  _p->_dict.commonPrefixSearch(buf, result, MAX_PREFIX_SEARCH_RESULT, key_len);
    rs._p->_reulst_idx = num;
    return num;
}
int
BaseDict::ExactMatchScript(const char* key, size_t key_len)
{
    Darts::DoubleArray::result_pair_type  rs;
    _p->_dict.exactMatchSearch(key, rs, key_len);
    //printf("query key=%s, len=%d, value= %d\n", key, rs.length, rs.value);
    return rs.value;
}


int
BaseDict::Insert(const char* term, unsigned int term_length, unsigned int term_id)
{
    if(!term_length)
        return -1;

    // build entry.
    LemmaEntry entry;
    entry.term = term;
    entry.term_id = term_id;

    CHECK(term_id) << "term id cann't be zero!";
    CHECK_LT(term_id, MAX_LEMMA_ENTRY_ID) << "term id larger than 2G!";
    CHECK_LT(term_length, MAX_ENTRY_TERM_LENGTH) << "term too long!";
    //_p->entries[entry.term_id] = entry;
    _p->AddEntry(entry); //copy construct
    return 0;
}

// EntryData
u4
BaseDict::GetEntryPropertyU4(u4 value, const char* key, u4 def_val)
{
    int data_size = 0;
    const void* dataptr = _p->GetOnDiskEntryProperty(value, &data_size);
    if(dataptr && data_size) {
        if(1) {
            // check type.
            char prop_type = _schema.GetColumnType(key);
            if(prop_type!='2' && prop_type!='4')
                return 0; // invalid type.
        } // end type check.
        return _p->GetOnDiskEntryPropertyU4(dataptr, data_size, key, def_val);
        //printf("value=%d, datasize=%d\n", value, data_size);
        //return 1;
    }
    return 0;  // data not found ?
}

const char*
BaseDict::GetEntryProperty(u4 value,  const char* key, int* data_len)
{
    int data_size = 0;
    const void* dataptr = _p->GetOnDiskEntryProperty(value, &data_size);
    if(dataptr && data_size) {
        if(1) {
            // check type.
            char prop_type = _schema.GetColumnType(key);
            if(prop_type != 's')
                return NULL; // invalid type.
        } // end type check.
        return _p->GetOnDiskEntryPropertyString(dataptr, data_size, key, data_len);
    }
    return NULL;
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

    CHECK_LT(data_len + 8 , MAX_ENTRY_DATA) << "property data too large!";

    LemmaPropertyEntry prop_entry;
    prop_entry.prop_name = key;
    {
        // lookup property key
        prop_entry.prop_type = _schema.GetColumnType(key);
        if(prop_entry.prop_type == ' ')
            return -2; // prop not found.
    }
    // set data.
    {
        void* data_ptr = &(prop_entry.data);
        switch (prop_entry.prop_type ) {
        case '2':{
                u2* ptr = (u2*)data;
                *(u2*)data_ptr = *ptr;
            }
            break;
        case '4':{
                u4* ptr = (u4*)data;
                *(u4*)data_ptr = *ptr;
            }
            break;
        case '8':{
                u4* ptr = (u4*)data;
                // 32bit compt
                *(u4*)data_ptr = *ptr;
                prop_entry.data_len = ptr[1] & 0xFFFFFFFF; //ensure only 32bit.
            }
            break;
        case 's':{
                prop_entry.data = malloc(data_len);
                memcpy(prop_entry.data, data, data_len);
                prop_entry.data_len = data_len;
            }
            break;
        default:
           CHECK(false) << "invalid entry_prop_type." << prop_entry.prop_type << "===";
        }
    }
    // add prop_entry -> term_entry.
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
    //FIXME: add type checking.
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
