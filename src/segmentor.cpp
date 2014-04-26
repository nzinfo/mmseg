#include <stdio.h>

#include "segmentor.h"

Segmentor::Segmentor()
{
}

Segmentor::~Segmentor()
{
}

int
Segmentor::LoadTermDictionary(const char* dict_path, int dict_id)
{
    return 0;
}

int
Segmentor::LoadPharseDictionary(const char* dict_path, int dict_id)
{
    return 0;
}

int
Segmentor::Tokenize(SegmentStatus* stat, const char *utf8_string, u4 utf8_string_len )
{
    printf("%s\n", utf8_string);
    return 0;
}

// -*- end of file -*-
