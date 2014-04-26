/*
 *  MMSeg Chinese tokenizer command line interface.
 */

#include <fstream>
#include <string>
#include <iostream>
#include <cstdio>
#include <algorithm>
#include <map>
#include  <stdlib.h>
#include <stdio.h>

#ifdef WIN32
#include "bsd_getopt_win.h"
#else
#include "bsd_getopt.h"
#endif

#define MMSEG_DEBUG 1

// mmseg includes
#include "segmentor.h"

void usage(const char* argv_0) {
    printf("Coreseek COS(tm) MMSeg 2.0\n");
    printf("Copyright By Coreseek.com All Right Reserved.\n");
    printf("Usage: %s <option> <file>\n",argv_0);
    printf("-d <dict_path>           Dictionary\n");
    printf("-h            print this help and exit\n");
    return;
}

int segment(const char* file, Segmentor& seg, u1 b_quit);

int main(int argc, char **argv) {

    char resolved_dict_path[255];
    int c;
    const char* dict_path = NULL;
    const char* out_file = NULL;
    u1 bQuite = 0;
    int rs = 0;

    Segmentor seg;

    if(argc < 2){
        usage(argv[0]);
        exit(0);
    }

    while ((c = getopt(argc, argv, "d:q")) != -1) {
        switch (c) {
        case 'd':
            dict_path = optarg;
            break;
        case 'q':
            bQuite = 1;
            break;
        case 'h':
            usage(argv[0]);
            exit(0);
        default:
            fprintf(stderr, "Illegal argument \"%c\"\n", c);
            return 1;
        }
    }

    if(optind < argc) {
        out_file = argv[optind];
    }

    realpath(dict_path, resolved_dict_path);

#if MMSEG_DEBUG
    printf("dict=%s; file=%s\n", resolved_dict_path, out_file);
#endif

    // load basic diction only -> for system booting up.
    {
        SegmentOptions opts;
        rs = seg.LoadTermDictionary(resolved_dict_path, 0, opts);
    }
    rs = segment(out_file, seg, bQuite);

    return 0;
}


int segment(const char* utf8_file, Segmentor& seg, u1 b_quit) {
    /*
     *  1 load all text from disk, file must encode in utf8
     *  2 strip BOM
     *  3 send it to seg, wait result.
     */
    int rs = 0;
    int length;
    char* buffer = NULL;
    char* buffer_ptr = NULL;
    char txtHead[3] = {239,187,191};

    // load buffer
    {
        std::ifstream is(utf8_file);
        //load data.
        is.seekg (0, std::ios::end);
        length = is.tellg();
        is.seekg (0, std::ios::beg);
        buffer = new char [length+1];
        is.read (buffer,length);
        buffer[length] = 0;

        buffer_ptr = buffer;
        // check header UTF-8 BOM
        if(memcmp(buffer,txtHead,sizeof(char)*3) == 0) {
            buffer_ptr += 3; // fast forward.
            length -= 3;
        }
    }
    // do segment
    {
        SegmentOptions opts;
        SegmentStatus seg_stat;
        rs = seg.Tokenize(&seg_stat, buffer, length, opts);
    }

    // free memory
    {
        delete[] buffer;
    }
    return 0;
}

/* end of file */
