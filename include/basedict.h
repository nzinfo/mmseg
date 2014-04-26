#ifndef BASEDICT_H
#define BASEDICT_H

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

enum LemmaPropertyType {
    PROP_STRING,        // variant size
    PROP_SHORT,         // 2bit
    PROP_INT,           // 4bit
    PROP_LONG           // 8bit
};

typedef struct LemmaPropertyDefine
{
    char key[64];
    LemmaPropertyType prop_type;
}LemmaPropertyDefine;

typedef struct LemmaPropertyEntry
{
    char* key;
    LemmaPropertyType prop_type;
    void* data;
    u4          data_len;
}LemmaPropertyEntry;


class BaseDict
{
public:
    BaseDict();
    ~BaseDict();

public:
    int Open(const char* dict_path, char mode); // mode can be 'r', 'n'.  'n' stands for new; 'r' load pre-build dict from disk.
    int Save(const char* dict_path);            // save to disk
    int Build();                                // build trie-tree in memory.

    int Init(const LemmaPropertyDefine* props, int prop_count);     // define how many propery a lemma in this dictionary can have.
                                                                    // once this func been call, all data in dict will be trunc

    int Insert(const char* term, int freq, const u4* pos, int pos_count); // add new term -> dict, pos = char[4]

    int SetProp(const char* term, const char* key, const void* data, int data_len); // when prop_type is short|int|long, data_len will be ignored.
    int GetProp(const char* term, const char* key, void** data, int* data_len);

    int Properties(const char* term, LemmaPropertyEntry** entries);

};

#endif // BASEDICT_H
