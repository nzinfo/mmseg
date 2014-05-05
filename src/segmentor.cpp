#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dirent.h"
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>

#include <glog/logging.h>

#include "segmentor.h"
#include "basedict.h"

////////////////////////////////////////////////////////////////////////////////
/// TermDartsIndex
///     用于同时查找多个词条的索引
////////////////////////////////////////////////////////////////////////////////

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
    }
    // load term
    {
        for(std::vector<std::string>::iterator it = term_dicts.begin(); it != term_dicts.end(); ++it) {
            //printf("tok=%s\t", it->key);

        }
    }
    // load pharse
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
    printf("%s\n", utf8_string);
    return 0;
}

// -*- end of file -*-
