#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dirent.h"
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <algorithm>
#include <queue>

#include <glog/logging.h>

#include "segmentor.h"
#include "basedict.h"

////////////////////////////////////////////////////////////////////////////////
/// TermDartsIndex
///     用于同时查找多个词条的索引
///     - Term
///     - Pharse
////////////////////////////////////////////////////////////////////////////////
class TermDartsIndexDictItem
{
public:
    short  _dict_id;
    u1ptr  _key;
    u4     _value;

public:
    // DONT change the following code... done by enum all condition.
    bool operator<(const TermDartsIndexDictItem& rhs) const {
        int n = strcmp((char*)this->_key, (char*)rhs._key);
        if (n == 0)
            return this->_dict_id > rhs._dict_id;
        //printf("cmp %s vs %s rs=%d\n", _key, rhs._key, n);
        return n > 0;
    }
};

class TermDartsIndexDictIterator
{
public:
    TermDartsIndexDictIterator():
        _dict_id(0), _dict_type(100), _dict_rev(0), _entry_count(0), _values(NULL), _keys(NULL) {
       //ctor
        _entry_pos = 0;
    }

public:
    short  _dict_id;
    short  _dict_type;      //  100 = term; 200 = pharse.
    std::string dict_name;
    u4     _dict_rev;

    u4     _entry_count;
    u4*    _values;
    u1ptr* _keys;

    u4     _entry_pos;

    TermDartsIndexDictItem next() {
        if(_entry_pos < _entry_count) {
            TermDartsIndexDictItem item = { _dict_id, _keys[_entry_pos], _values[_entry_pos] };
            _entry_pos ++;
            return item;
        } else {
            TermDartsIndexDictItem item = { _dict_id, NULL, 0 };
            return item;
        } // end if
    }

    /*
    bool operator<(const TermDartsIndexDictIterator& rhs) const {
        int n = strcmp(this->_key, rhs._key);
        if (n == 0)
            return this->_dict_id < rhs._dict_id;
        return n < 0;
    }*/
public:
    const static short DT_TERM = 100;
    const static short DT_PHARSE = 200;
};

class TermDartsIndexValueEntry
{
public:
    TermDartsIndexValueEntry():
        dict_flag(0) {}

public:
    u4 dict_flag;
    std::vector<u4> values;

    int AddDict(short dict_id, u4 value) {
        return 0;
    }

    int GetSize() {
        return sizeof(u4) + sizeof(u4)*values.size();
    }
};

class TermDartsIndex
{

public:
    int SaveCache(const char* filename) {
        /*
         *  词库格式
         *   Header:
         *      MGC
         *      VER
         *      count of terms
         *      count of pharse
         *      length of <id, dict> mapping
         *
         *  Terms:
         *      ( id, length ) : dict_unique_name
         *  Pharse:
         *      ....
         *
         *  Darts
         *      ...
         *  Values Pool
         *      value = flag, [list of values]
         * 1 检测文件是否 可写
         * 2
         */
        return 0;
    }

    int LoadCache(const char* filename) {
        return 0;
    }

    int RevokeCache() {
        /*
         *  废弃已经加载的 Cache
         *
         */
        return 0;
    }

    int VerifyCache(mm::BaseDict* term_dicts, int term_dict_size, mm::PharseDict* pharse_dicts, int pharse_dict_size)
    {
        /*
         *  校验目前的词条与已经加载的词条版本是否匹配:
         *      - 检测 词库的ID & 词库的 Rev 号  (因为有可能定时重建, 因此不能检测创建时间. Rev 号检测基于约定)
         */
        return 0;
    }

    int Build(mm::BaseDict** term_dicts, int term_dict_size,
              mm::PharseDict** pharse_dicts, int pharse_dict_size)
    {
        /*
         *  合并全部的 term_dict & pharse_dict 构成一统一的词条
         *  读取全部的词条, 构成统一的索引
         *      key -> values 对;
         *
         * 词典总数不能超过 32, 因为使用一个 32bit 作为 flag.
         *
         *  1 计算全部的 词条数
         *  2 计算需要分配的 value_pool 的最大值
         *  3 使用 heap sort 对需要索引的词条进行排序
         *  4 构建索引 & 保存
         */
        int iTotalSize = 0;
        int rs = 0;

        TermDartsIndexDictIterator DictIter[MM_SEGMENTOR_MAX_DICTIONARY];
        CHECK_LE(MM_SEGMENTOR_MAX_DICTIONARY, 32) << " dictionary more than 32.";

        TermDartsIndexDictIterator* DictIterPtr = DictIter;

        // get all length
        // do term
        for(int i=0; i< term_dict_size; i++) {
            mm::BaseDict* dict = term_dicts[i];
            if(!dict)
                continue;
            rs = dict->GetOnDiskDictionaryRawData((*DictIterPtr)._entry_count, (*DictIterPtr)._keys, (*DictIterPtr)._values);
            if(rs)
                CHECK_EQ(rs, 0) << " fail to get term raw data." << dict->GetDictName();
            (*DictIterPtr).dict_name = dict->GetDictName();
            (*DictIterPtr)._dict_rev = dict->GetDictRev();
            (*DictIterPtr)._dict_id = DictIterPtr - DictIter; // 30, 31 dict id reversed.
            (*DictIterPtr)._dict_type = TermDartsIndexDictIterator::DT_TERM;
            iTotalSize += (*DictIterPtr)._entry_count;

            DictIterPtr ++;
        }
        // do pharse
        for(int i=0; i< pharse_dict_size; i++) {
            mm::PharseDict* dict = pharse_dicts[i];
            if(!dict)
                continue;
            rs = dict->GetOnDiskDictionaryRawData((*DictIterPtr)._entry_count, (*DictIterPtr)._keys, (*DictIterPtr)._values);
            if(rs)
                CHECK_EQ(rs, 0) << " fail to get pharse raw data." << dict->GetDictName();

            (*DictIterPtr).dict_name = dict->GetDictName();
            (*DictIterPtr)._dict_rev = dict->GetDictRev();
            (*DictIterPtr)._dict_id = DictIterPtr - DictIter;  // 30, 31 dict id reversed.
            (*DictIterPtr)._dict_type = TermDartsIndexDictIterator::DT_PHARSE;
            iTotalSize += (*DictIterPtr)._entry_count;

            DictIterPtr ++;
        }

        LOG(INFO) << " build dart entry count " << iTotalSize;

        // create memory struct
        u1ptr* darts_keys = new u1ptr[iTotalSize];
        u4*    darts_values = new u4[iTotalSize];
        memset(darts_keys,0, sizeof(u1ptr)*iTotalSize);
        memset(darts_values,0, sizeof(u4)*iTotalSize);
        u1ptr* darts_keys_ptr = darts_keys;
        u4*    darts_values_ptr = darts_values;

        std::vector<TermDartsIndexValueEntry> entries;
        entries.reserve(iTotalSize);
        /* get all items
         *  使用堆排序, 对 darts 的构成进行预分配; 排序规则为 字母序  ASC, 词典编号序 ASC
         *  1 同一个词条存在与多个词库中, 此时不分配新的 key, 仅仅在 entries 增加新的记录
         *  2 如果从 que 中 出列了一个 term, 则从这个 term 的 dict 中选取一个词条.
         *  3 如果 某个 dict 已经没有新的词条, 则选择第一个还有数据的词条;
         *  4 如果全部词典都无数据, 排序完成
         */
        {
            std::priority_queue<TermDartsIndexDictItem> que;
            // each dict push one entry.
            for(int i=0; i< DictIterPtr - DictIter; i++) {
                TermDartsIndexDictItem item = DictIter[i].next();
                //printf("push .. key=%s, value=%d, dict=%d\n", item._key, item._value, item._dict_id);
                que.push(item);
            }

            for(int i=0; i< iTotalSize && !que.empty(); i++){
                // on each term.
                TermDartsIndexDictItem item  = que.top();
                que.pop();
                // check preq is eq
                if(*darts_keys_ptr == NULL)
                    *darts_keys_ptr = item._key;

                if(strcmp((char*)item._key, (char*)darts_keys_ptr[0]) == 0) {
                    // existed term
                    entries.back().AddDict(item._dict_id, item._value);
                }else{
                    //printf("pop key=%s, value=%d, dict=%d, que size = %d\n", item._key, item._value, item._dict_id, que.size());
                    TermDartsIndexValueEntry entry;
                    // new term
                    *darts_values_ptr = entries.back().GetSize();
                    // move to next entry
                    darts_keys_ptr ++;
                    darts_values_ptr ++;
                    *darts_keys_ptr = item._key;
                    *darts_values_ptr = 0;

                    entries.push_back(entry);
                    entries.back().AddDict(item._dict_id, item._value);
                }
                // check is end
                item = DictIter[item._dict_id].next();
                //printf("next key=%s, value=%d, dict=%d, que size = %d\n", item._key, item._value, item._dict_id, que.size());
                if(item._key != NULL)
                    que.push(item);
            }
        }


        for(int i=0; i< DictIterPtr - DictIter; i++)  {
            for(int j=0; j< DictIter[i]._entry_count; j++) {
                printf("key=%s, len=%d, val= %d \n", DictIter[i]._keys[j], strlen((char*)DictIter[i]._keys[j]), DictIter[i]._values[j]);
                if(j>10)
                    break;
            }
        }


        {
            // key, value source_dict_id
              int myints[] = {10,20,30,5,15};
              std::vector<int> v(myints,myints+5);

              std::make_heap (v.begin(),v.end());
              std::cout << "initial max heap   : " << v.front() << '\n';

              std::pop_heap (v.begin(),v.end()); v.pop_back();
              std::cout << "max heap after pop : " << v.front() << '\n';

              v.push_back(99); std::push_heap (v.begin(),v.end());
              std::cout << "max heap after push: " << v.front() << '\n';

              std::sort_heap (v.begin(),v.end());

              std::cout << "final sorted range :";
              for (unsigned i=0; i<v.size(); i++)
                std::cout << ' ' << v[i];

              std::cout << '\n';
        }
        printf("cccc");
        return 0;
    }

protected:
    short _cache_dict_id2_segment_dict_id[32];
};

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
    :_char_mapper(NULL)
    ,_term_index(NULL)
{
    memset(_term_dicts, 0, sizeof(_term_dicts));
    memset(_pharse_dicts, 0, sizeof(_pharse_dicts));
}

Segmentor::~Segmentor()
{
    Reset();
}

void
Segmentor::Reset()
{
    mm::BaseDict** term_dict_ptr = _term_dicts;
    while(*term_dict_ptr) {
        mm::BaseDict* dict = *term_dict_ptr;
        if(dict)
            delete dict;
        term_dict_ptr ++;
    }
    memset(_term_dicts, 0, sizeof(_term_dicts));

    mm::PharseDict** phs_dict_ptr = _pharse_dicts;
    while(*phs_dict_ptr) {
        mm::PharseDict* dict = *phs_dict_ptr;
        if(dict)
            delete dict;
        phs_dict_ptr ++;
    }
    memset(_pharse_dicts, 0, sizeof(_pharse_dicts));

    if(_char_mapper) {
        delete _char_mapper;
        _char_mapper = NULL;
    }
}

int
Segmentor::LoadDictionaries(const char* dict_path, SegmentOptions &opts)
{
    /*
     *  从一个目录中加载词典
     *  - charmap           base.chr        只能有一个 charmap
     *  - term              *.uni
     *  - pharse            *.phs
     *
     *  并构造 darts 索引的 Cache
     *  1 按字母顺序加载, 给系统提供一个额外的控制加载顺序的机制
     *
     *  -404 path not found.
     */
    mm::CharMapper* mapper = new mm::CharMapper();
    mm::BaseDict* new_term_dicts[MM_SEGMENTOR_MAX_TERM_DICTIONARY];
    mm::BaseDict* new_pharse_dicts[MM_SEGMENTOR_MAX_PHARSE_DICTIONARY];

    memset(new_term_dicts, 0, sizeof(new_term_dicts));
    memset(new_pharse_dicts, 0, sizeof(new_pharse_dicts));

    int rs = 0;

    rs = LoadDictionaries(dict_path, mapper, new_term_dicts, new_pharse_dicts);
    if(rs < 0)
        return rs;

    // set to current dictionary
    {
        Reset();
        // reassign
        _char_mapper = mapper;
        memcpy(_term_dicts, new_term_dicts, sizeof(new_term_dicts));
        memcpy(_pharse_dicts, new_pharse_dicts, sizeof(new_pharse_dicts));

        // rebuild index
        BuildIndex();
    }
    return 0;
}

int
Segmentor::LoadDictionaries(const char* dict_path, mm::CharMapper* mapper, mm::BaseDict** terms, mm::BaseDict** pharses)
{
    std::string fname; fname.reserve(255);
    std::string unichar;        // 如果没有 base.chr 但是有另外一个 chr 扩展名的文件?  ... 目前如果没有, 则 crash

    std::vector<std::string> charmap_dicts;         charmap_dicts.reserve(10);
    std::vector<std::string> term_dicts;            term_dicts.reserve(30);
    std::vector<std::string> pharse_dicts;          pharse_dicts.reserve(10);

    DIR *dir;
    struct stat filestat;
    struct dirent *ent;
    if ((dir = opendir (dict_path)) != NULL) {
      /* print all the files and directories within directory */
      while ((ent = readdir (dir)) != NULL) {
          fname = dict_path;
          fname += "/";
          fname +=ent->d_name;
          if (stat( fname.c_str(), &filestat )) continue;
          if (S_ISDIR( filestat.st_mode ))         continue;

          if( std::string::npos != fname.rfind(".uni", fname.length() - 4, 4) ) {
              term_dicts.push_back(fname);
          }
          if( std::string::npos != fname.rfind(".phs", fname.length() - 4, 4) ) {
              pharse_dicts.push_back(fname);
          }
          if( std::string::npos != fname.rfind(".chr", fname.length() - 4, 4) ) {
              charmap_dicts.push_back(fname);
          }
      } // end while
      closedir (dir);
    } else {
      return -404;
    }
    // sort
    std::sort(term_dicts.begin(), term_dicts.end());
    std::sort(pharse_dicts.begin(), pharse_dicts.end());
    std::sort(charmap_dicts.begin(), charmap_dicts.end());

    // check
    CHECK_GT(charmap_dicts.size(), 0) << "charmap file not found.";
    CHECK_GT(term_dicts.size(), 0) << "should have at lease on term dictionary.";

    CHECK_LT(term_dicts.size(), MM_SEGMENTOR_MAX_TERM_DICTIONARY) << "term dictionary more than "<< MM_SEGMENTOR_MAX_TERM_DICTIONARY;
    CHECK_LT(pharse_dicts.size(), MM_SEGMENTOR_MAX_PHARSE_DICTIONARY) << "term dictionary more than "<< MM_SEGMENTOR_MAX_TERM_DICTIONARY;

    // load unichar
    {
        /*
         * 1 if base.chr exist, load it
         * 2 if not, load the 1st
         */
        std::string char_map_fname = "base.chr";
        std::vector<std::string>::iterator it = find(charmap_dicts.begin(), charmap_dicts.end(), "base.chr");
        if(it == charmap_dicts.end()) {
            // not found
            char_map_fname = *it;
        }
        LOG(INFO) << " load charmap " << char_map_fname;
        mapper->Load(char_map_fname.c_str());
    }
    // load term
    {
        int n = 0;
        for(std::vector<std::string>::iterator it = term_dicts.begin(); it != term_dicts.end(); ++it) {
            //printf("tok=%s\t", it->key);
            std::string& fname = *it;
            LOG(INFO) << " load terms " << fname;
            mm::BaseDict * dict = new mm::BaseDict();
            n = dict->Load(fname.c_str(), 'c');
            if(n < 0) {
                LOG(INFO) << " load terms " << fname << " error with " << n;
            }
            *terms = dict;
            terms++;
        }
    }
    // load pharse
    {
        int n = 0;
        for(std::vector<std::string>::iterator it = pharse_dicts.begin(); it != pharse_dicts.end(); ++it) {
            //printf("tok=%s\t", it->key);
            std::string& fname = *it;
            LOG(INFO) << " load pharse " << fname;
            mm::PharseDict * dict = new mm::PharseDict();
            n = dict->Load(fname.c_str(), 'c');
            if(n < 0) {
                LOG(INFO) << " load pharse " << fname << " error with " << n;
            }

            *terms = dict;
            terms++;
        }
    }
    return 0;
}

int
Segmentor::LoadCharmapDictionary(const char* dict_file, SegmentOptions& opts)
{
    return 0;
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
Segmentor::BuildIndex()
{
    int rs = _term_index->Build(this->_term_dicts, MM_SEGMENTOR_MAX_TERM_DICTIONARY,
                                this->_pharse_dicts, MM_SEGMENTOR_MAX_PHARSE_DICTIONARY);

    return rs;
}

/*
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
*/

int
Segmentor::ReloadDictionary(int dict_id)
{
    return 0;
}

/*
int
Segmentor::ReplaceDictionary(const mm::BaseDict* dict, int dict_id)
{
    return 0;
}
*/

int
Segmentor::Tokenize(SegmentStatus* stat, const char *utf8_string, u4 utf8_string_len, SegmentOptions& opts)
{
    /*
     *  if stat not set , use utf8_string init stat, else , ignore utf8_string
     */
    //printf("%s\n", utf8_string);
    return 0;
}

// -*- end of file -*-
