#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "segmentor.h"
#include "basedict.h"

////////////////////////////////////////////////////////////////////////////////
SegmentStatus::SegmentStatus()
    :utf8_buf(NULL)
    ,utf8_buf_ptr(NULL)
    ,utf8_buf_length(0)
{
    memset(icode_buf, 0, sizeof(u4)*SEGMENT_ICODE_BUFFER_LENGTH);
    memset(icode_buf_lower, 0, sizeof(u4)*SEGMENT_ICODE_BUFFER_LENGTH);
    memset(icode_type, 0, sizeof(u1)*SEGMENT_ICODE_BUFFER_LENGTH);
}

void
SegmentStatus::Reset()
{
    utf8_buf = utf8_buf_ptr = NULL;
    utf8_buf_length = 0;

    memset(icode_buf, 0, sizeof(u4)*SEGMENT_ICODE_BUFFER_LENGTH);
    memset(icode_buf_lower, 0, sizeof(u4)*SEGMENT_ICODE_BUFFER_LENGTH);
    memset(icode_type, 0, sizeof(u1)*SEGMENT_ICODE_BUFFER_LENGTH);
}

////////////////////////////////////////////////////////////////////////////////
/// Segmentor Private
////////////////////////////////////////////////////////////////////////////////
class SegmentorPrivate
{
public:

};


////////////////////////////////////////////////////////////////////////////////
Segmentor::Segmentor()
{
}

Segmentor::~Segmentor()
{
}

int
Segmentor::LoadTermDictionary(const char* dict_path, int dict_id, SegmentOptions& opts)
{
    return 0;
}

int
Segmentor::LoadPharseDictionary(const char* dict_path, int dict_id, SegmentOptions& opts)
{
    return 0;
}

int
Segmentor::AddTermDictionary(const mm::BaseDict* dict, int dict_id, SegmentOptions& opts)
{
    return 0;
}

int
Segmentor::AddPharseDictionary(const mm::BaseDict* dict, int dict_id, SegmentOptions& opts)
{
    return 0;
}

int
Segmentor::ReloadDictionary(int dict_id)
{
    return 0;
}

int
Segmentor::ReplaceDictionary(const mm::BaseDict* dict, int dict_id)
{
    return 0;
}

int
Segmentor::Tokenize(SegmentStatus* stat, const char *utf8_string, u4 utf8_string_len, SegmentOptions& opts)
{
    /*
     *  if stat not set , use utf8_string init stat, else , ignore utf8_string
     */
    printf("%s\n", utf8_string);
    return 0;
}

// -*- end of file -*-
