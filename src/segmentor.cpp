#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dirent.h"
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>

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
    std::vector<std::string> term_dicts;
    std::vector<std::string> pharse_dicts;

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
          {
                printf ("%s\n", ent->d_name);
          }
      } // end while
      closedir (dir);
    } else {
      return -404;
    }

    // load unichar
    // load term
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
