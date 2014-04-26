#include "basedict.h"

BaseDict::BaseDict()
{
}

BaseDict::~BaseDict()
{
}

////////////////////////////////////////////////////////////////////////////////
int
BaseDict::Open(const char* dict_path, char mode)
{
    return 0;
}

int
BaseDict::Save(const char* dict_path){
    return 0;
}

int
BaseDict::Build()
{
    return 0;
}

int
BaseDict::Init(const LemmaPropertyDefine* props, int prop_count)
{
    return 0;
}

int
BaseDict::Insert(const char* term, int freq, const u4* pos, int pos_count)
{
    return 0;
}

int
BaseDict::SetProp(const char* term, const char* key, const void* data, int data_len)
{
    return 0;
}

int
BaseDict::GetProp(const char* term, const char* key, void** data, int* data_len)
{
    return 0;
}

int
BaseDict::Properties(const char* term, LemmaPropertyEntry** entries){
    return 0;
}

/* -- end of file -- */
