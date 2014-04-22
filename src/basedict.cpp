#include "basedict.h"

BaseDict::BaseDict()
{
}

BaseDict::~BaseDict()
{
}

////////////////////////////////////////////////////////////////////////////////
int
BaseDict::open(const char* dict_path, char mode)
{
    return 0;
}

int
BaseDict::save(const char* dict_path){
    return 0;
}

int
BaseDict::build()
{
    return 0;
}

int
BaseDict::init(const LemmaPropertyDefine* props, int prop_count)
{
    return 0;
}

int
BaseDict::insert(const char* term, int freq, const u4* pos, int pos_count)
{
    return 0;
}

int
BaseDict::setprop(const char* term, const char* key, const void* data, int data_len)
{
    return 0;
}

int
BaseDict::getprop(const char* term, const char* key, void** data, int* data_len)
{
    return 0;
}

int
BaseDict::properties(const char* term){
    return 0;
}

