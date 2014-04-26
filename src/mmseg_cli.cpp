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


int main(int argc, char **argv) {

    char resolved_dict_path[255];
    int c;
    const char* dict_path = NULL;
    const char* out_file = NULL;
    u1 bQuite = 0;

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


    return 0;
}

/* end of file */
