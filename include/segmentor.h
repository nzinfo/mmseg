#ifndef SEGMENTOR_H
#define SEGMENTOR_H

#include "csr_typedefs.h"

/*
 *  Define How to use the tokenizer.
 */
class SegmentStatus
{
    /*
     *  1 Record token's status, all person names, etc
     *  2 Record current Status  ( init, inprogess, finished )
     *  3
     */
};

class Segmentor
{
public:
    Segmentor();
    ~Segmentor();

public:
    int LoadTermDictionary(const char* dict_path, int dict_id);
    int LoadPharseDictionary(const char* dict_path, int dict_id);

    int Tokenize(SegmentStatus* stat, const char *utf8_string, u4 utf8_string_len );
};

#endif // SEGMENTOR_H
