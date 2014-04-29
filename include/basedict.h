#ifndef BASEDICT_H
#define BASEDICT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
#define MAX_UNICODE_CODEPOINT   65535       // UCS-2's MAX
#define MAX_LEMMA_PROPERTY_NAME_LENGTH 64
#define MAX_LEMMA_ENTRY_ID      2147483648  //2^31
#define MAX_PROPERTY_COUNT      15          //if fact can not change, the highest bit stands for ID | DATA
#define MAX_ENTRY_DATA          65535       //64K
#define MAX_ENTRY_TERM_LENGTH   32768       //2**15 max term length

namespace mm {

/*
enum LemmaPropertyType {
    PROP_STRING,        // s: variant size (4B offset, 2G Max)
    PROP_SHORT,         // 2: 2B
    PROP_INT,           // 4: 4B
    PROP_LONG           // 8: 8B
}; */

typedef struct LemmaPropertyDefine
{
    char prop_type;         // s 2 4 8
    char key[MAX_LEMMA_PROPERTY_NAME_LENGTH];
}LemmaPropertyDefine;

class LemmaPropertyEntry;
class BaseDictPrivate;

class BaseDict
{
public:
    BaseDict();
    ~BaseDict();

public:
    int Load(const char* dict_path, char mode); // mode can be 'r', 'n'.  'n' stands for new; 'r' load pre-build dict from disk.
    int Save(const char* dict_path, int dict_rev);            // save to disk
    int Build();                                // build trie-tree in memory.
    int Reset();                                // clear all in memory entry (inlcude property's data)

    int Init(const LemmaPropertyDefine* props, int prop_count);     // define how many propery a lemma in this dictionary can have.
                                                                    // once this func been call, all data in dict will be trunc
    /*
     *  property_name:type;
     *  Default: id:u4; comunicate with external system?
     */
    int InitString(const char* prop_define, int str_define_len);    // use string define property, for scripting interface.
    int SetDictName(const char* dict_name);

    int Insert(const char* term, unsigned int term_length, unsigned int term_id, int freq); // add new term -> dict, pos = char[4]

    int SetProp(unsigned int term_id, const char* key, const char* data, int data_len); // when prop_type is short|int|long, data_len will be ignored.
    int GetProp(unsigned int term_id, const char* key, char* data, int* data_len);
    int SetPropInteger(unsigned int term_id, const char* key, u8 v);
    int GetPropInteger(unsigned int term_id, const char* key, u8* v);

    //int Properties(const char* term, LemmaPropertyEntry** entries);

    // Darts Debug
    int SaveRaw(const char* dict_path);
    int LoadRaw(const char* dict_path);

private:
    BaseDictPrivate* _p;
};

class CharMapper
{
    /*
     * Convert char -> simp | sym case. eg. A->a; 0xF900 -> 0x8c48
     * Get char's category number. ascii...
     *
     * 256K in memory by default
     */
public:
    CharMapper(bool default_pass=false):_bDefaultPass(default_pass) {
        memset(_char_mapping, 0, sizeof(u4)*MAX_UNICODE_CODEPOINT);
    }

public:
    int Load(const char* filename);
    int Save(const char* filename);

    // mapping opt, not support A..Z/2 , should be done @ script side.
    int Mapping(u4 src, u4 dest, u2 tag = 0);
    int MappingRange(u4 src_begin, u4 src_end, u4 dest_begin, u4 dest_end, u2 tag = 0);
    int MappingPass(u4 src_begin, u2 tag = 0);
    int MappingRangePass(u4 src_begin, u4 src_end, u2 tag = 0);

    inline u4 Transform(u4 src, u2* out_tag);
    u4 TransformScript(u4 src, u2* out_tag);  // the script side , check for trans

private:
    u4   _char_mapping[MAX_UNICODE_CODEPOINT]; // lower bit is trans iCode, higher is category flag.
    bool _bDefaultPass;
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
