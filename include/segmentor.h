#ifndef SEGMENTOR_H
#define SEGMENTOR_H

#include "csr_typedefs.h"

/*
 *  Segment Options , define switch-set during dictionary loading & segment.
 */
class SegmentOptions {
public:
    SegmentOptions():
        opt1(false)
    {}

public:
    bool opt1;
};

/*
 *  Define How to use the tokenizer.
 *  - use this class to avoid dyn malloc during segment.
 */
#define SEGMENT_ICODE_BUFFER_LENGTH     4096

class SegmentStatus
{
    /*
     *  1 Record token's status, all person names, etc
     *  2 Record current Status  ( init, inprogess, finished )
     *  3 Save the result.
     */
public:
    SegmentStatus();

public:
    void Reset();

public:
    const char* utf8_buf; //borrow data buffer;
    const char* utf8_buf_ptr; // current pointer.
    u4          utf8_buf_length;

    // icode buffer
    u4          icode_buf[SEGMENT_ICODE_BUFFER_LENGTH];        // use UTF-32 save charactor's icode.
    u4          icode_buf_lower[SEGMENT_ICODE_BUFFER_LENGTH];  // lower case tradtion -> simpl ? [NO!] 繁简转换应该在更高层面处理
    u1          icode_type[SEGMENT_ICODE_BUFFER_LENGTH];       // char's flag. only lower 64K char have flag.

    // seg result.
};

class Token
{
    /*
     * The token wrapper , client library access segment result.
     */
    int GetToken();             // the lower case format.
    int GetTokenOrigin();       // origin
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
    int LoadTermDictionary(const char* dict_path, int dict_id, SegmentOptions& opts);
    int LoadPharseDictionary(const char* dict_path, int dict_id, SegmentOptions& opts);

    int Tokenize(SegmentStatus* stat, const char *utf8_string, u4 utf8_string_len, SegmentOptions& opts);
};

#endif // SEGMENTOR_H
