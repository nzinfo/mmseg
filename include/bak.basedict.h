#ifndef BASEDICT_H
#define BASEDICT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <sstream>
#ifdef __APPLE__
#include <unordered_map>
using std::unordered_map;
#else
    #ifdef __GNUC__
    #include <tr1/unordered_map>
    using namespace std::tr1;
    //#include <ext/hash_map>
    //   #define unordered_map std::hash_map
    #endif
#endif

/*
 *  Basic Dictionary Interface for Tokenizer.
 *
 *  * Define dictionary
 *  * Import
 *  * Add new lemma
 *  * Set lemma property
 *  * Load from dict
 *  * Build & Save new dict
 *
 *
 *  Due to dictionary will be wrap in swig, use plain c type in function.
 */
#include "csr_typedefs.h"

// CharMapper define
#define MAX_UNICODE_CODEPOINT   0x10FFFF      // Unicode Max
#define UNICODE_MASK            0x1FFFFF      // (2**21-1)
#define UNICODE_BITS            21            // Unicode's bits count.
#define UNICODE_PAGE_SIZE       0xFFFF        // 65536

#define MAX_LEMMA_PROPERTY_NAME_LENGTH 64
#define MAX_LEMMA_ENTRY_ID      2147483648  //2^31
#define MAX_PROPERTY_COUNT      15          //if fact can not change, the highest bit stands for ID | DATA
#define MAX_ENTRY_DATA          65535       //64K
#define MAX_ENTRY_TERM_LENGTH   32768       //2**15 max term length
#define MAX_PREFIX_SEARCH_RESULT    32      // 前缀搜索 最多返回 32 个结果


namespace mm {

/*
enum LemmaPropertyType {
    PROP_STRING,        // s: variant size (4B offset, 2G Max)
    PROP_SHORT,         // 2: 2B
    PROP_INT,           // 4: 4B
    PROP_LONG           // 8: 8B
}; */

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
};

////////////////////////////////////////////////////////////////////////////////

typedef struct LemmaPropertyDefine
{
    char prop_type;         // s 2 4 8
    char key[MAX_LEMMA_PROPERTY_NAME_LENGTH];
}LemmaPropertyDefine;

class LemmaPropertyEntry;
class BaseDictPrivate;

class BaseDictSchema
{
public:
    // Column Related
    int AddColumn(const std::string& def, u2 idx);

    void ResetColumn() {
        column_by_idx.clear();
        column_type_and_idx.clear();
    }

    inline char GetColumnType(int idx);

    inline char GetColumnType(const char* column_name) {
        unordered_map<std::string, u2>::iterator it = column_type_and_idx.find(column_name);
        if(it != column_type_and_idx.end() ) {
            return (it->second) & 0xFF; // give the lower byte
        }
        return ' ';  // type not found.
    }

    inline short GetColumnIdx(const char* column_name) {
        // FIXME: use a hash ?
        unordered_map<std::string, u2>::iterator it = column_type_and_idx.find(column_name);
        if(it != column_type_and_idx.end() )
            return (it->second) >> 8; // give the lower byte
        return -1;  // type not found.
    }

    const std::string GetSchemaDefine() ;

    int Init(const LemmaPropertyDefine* props, int prop_count)
    {
        /*
         * Calc & Build PropData Define.
         *  - combine u2 together.
         *  crc32(key): offset_in_data_row  u2 first; add together
         */
        char buf[MAX_LEMMA_PROPERTY_NAME_LENGTH+2];
        const LemmaPropertyDefine* props_ptr = props;
        // do not use _record_row_size , each row have dyn row data.
        for(int i=0; i<prop_count; i++) {
            // check is id defined in schema string;
            //CHECK_EQ(props_ptr[i].key, "id") << "You can't define column named id, system reversed!";
            snprintf(buf, sizeof(buf), "%c:%s", props_ptr[i].prop_type, props_ptr[i].key);
            AddColumn(buf, i);
        }
        return 0;
    }
    int InitString(const char* prop_define, int str_define_len);
protected:
    //unordered_map<std::string, int> column_offset;  // column-> index[2]|offset[2b] in data block
    unordered_map<int, std::string> column_by_idx;  // idx -> column name
    unordered_map<std::string, u2> column_type_and_idx;   // raw_name -> column type(1B), column index(1B),

};

class BaseDict;
class DictMatchResultPrivate;
class DictMatchResult
{
friend class BaseDict;

public:
    DictMatchResult();
    virtual ~DictMatchResult();

    /*
     * 根据编号获取实际的词条ID 和 属性数据
     */
    //const u1*   GetEntryData(int value, u4 &id, u4& data_len); //应该由词库提供本功能
    u8             GetResult();  // 取最后条记录
    u8             GetResult(int idx); // for script use only.
    inline void*   GetResultPtr();

    //int             GetResult(u8* );
    inline void     ClearResult();

    inline int      AddResult(u4 length, u4 value);

protected:
    DictMatchResultPrivate* _p;
};

class BaseDict
{
public:
    BaseDict();
    virtual ~BaseDict();

public:
    // mode can be 'r', 'c'.  'c' stands for combine,
    // when load with combind, darts idx will not built;
    // 'r' raw & only work as origin mmseg
    int Load(const char* dict_path, char mode);

    int Save(const char* dict_path, int dict_rev);            // save to disk
    int Build();                                // build trie-tree in memory.
    int Reset();                                // clear all in memory entry (inlcude property's data)

    int Init(const LemmaPropertyDefine* props, int prop_count);     // define how many propery a lemma in this dictionary can have.
                                                                    // once this func been call, all data in dict will be trunc
    /*
     *  property_name:type;
     *  Default: id:u4; comunicate with external system?
     */
    int InitString(const char* dict_name, const char* prop_define, int str_define_len);    // use string define property, for scripting interface.

    const std::string& GetDictName();
    u4 GetDictRev();

    int Insert(const char* term, unsigned int term_length, unsigned int term_id); // add new term -> dict, pos = char[4]

    int SetProp(unsigned int term_id, const char* key, const char* data, int data_len); // when prop_type is short|int|long, data_len will be ignored.
    int GetProp(unsigned int term_id, const char* key, char* data, int* data_len);
    int SetPropInteger(unsigned int term_id, const char* key, u8 v);
    int GetPropInteger(unsigned int term_id, const char* key, u8* v);

    //int Properties(const char* term, LemmaPropertyEntry** entries);

    // Darts Debug
    int SaveRaw(const char* dict_path);
    int LoadRaw(const char* dict_path);

    // Search
    int ExactMatch(const char* buf, size_t key_len, DictMatchResult& rs);
    int PrefixMatch(const char* buf, size_t key_len, DictMatchResult& rs);

    // Script Interface
    int ExactMatchScript(const char* key, size_t key_len);

    // EntryData, for Scripting..
    u4  GetEntryPropertyU4(u4 value, const char* key, u4 def_val);
    const char* GetEntryProperty(u4 value,  const char* key, int* data_len);  // 读取字符串信息

    /*
    const void* GetEntryData(u4 value, u4& term_id, u4& entry_date_len);
    // EntryData Script Interface
    u2  GetEntryPropertyU2(u4 value, const char* key);
    u4  GetEntryPropertyU4(u4 value, const char* key);
    u8  GetEntryPropertyU8(u4 value, const char* key);
    */

    // internal use
    int GetOnDiskDictionaryRawData(u4& nSize, u1ptr* & keys, u4* & values);   //读取从磁盘中加载的词库的原始数据.

protected:
    static char _head_mgc[5];
    virtual const char* get_file_head_mgc() {
        return _head_mgc;
    }

private:
    BaseDictPrivate* _p;
    BaseDictSchema _schema;
};

class PharseDict : public BaseDict
{

protected:
    static char _head_mgc[5];
    virtual const char* get_file_head_mgc() {
        return _head_mgc;
    }
};

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

} //end namespace

#endif // BASEDICT_H
