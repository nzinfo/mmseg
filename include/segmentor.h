#ifndef SEGMENTOR_H
#define SEGMENTOR_H

#include "csr_typedefs.h"

/*
 *  Define How to use the tokenizer.
 *  - use this class to avoid dyn malloc during segment.
 */
class SegmentStatus
{
    /*
     *  1 Record token's status, all person names, etc
     *  2 Record current Status  ( init, inprogess, finished )
     *  3 Save the result.
     */
};


class Segmentor
{
public:
    Segmentor();
    ~Segmentor();

public:
    /*
     *      char mapping   _char.map
     *      term    append _term.dict
     *      pharse  append _pharse.dict
     *  Can load multi-dictionary, but dict_id must be unique.
     */
    int LoadTermDictionary(const char* dict_path, int dict_id);
    int LoadPharseDictionary(const char* dict_path, int dict_id);

    int Tokenize(SegmentStatus* stat, const char *utf8_string, u4 utf8_string_len );
};

#endif // SEGMENTOR_H
