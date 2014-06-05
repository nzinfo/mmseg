/*
 *  MMSeg Chinese tokenizer command line interface.
 */
#include <glog/logging.h>
#include <fstream>
#include <string>
#include <iostream>
#include <cstdio>
#include <algorithm>
#include <map>
#include <stdlib.h>
#include <stdio.h>
#include <gflags/gflags.h>

extern "C" {
#if WIN32
#include "win32/dirent.h"
#include "win32/realpath_win32.h"
#endif
} // end extern C

#define MMSEG_DEBUG 1

#include "csr_typedefs.h"

#include "mm_dict_mgr.h"
#include "mm_seg_option.h"
#include "mm_seg_script.h"

#include "mm_segmentor.h"
#include "utils/pystring.h"

// mmseg includes
//#include "segmentor.h"

/*
void usage(const char* argv_0) {
    printf("Coreseek COS(tm) MMSeg 2.0\n");
    printf("Copyright By Coreseek.com All Right Reserved.\n");
    printf("Usage: %s <option> <file>\n",argv_0);
    printf("-d <dict_path>           Dictionary\n");
    printf("-h            print this help and exit\n");
    return;
}
*/

DEFINE_string(dict_path, ".", "where to load dictionary");
DEFINE_string(script_path, ".", "where to load scripts");

int segment(const char* file, const char* dict_path, const char* , u1 b_quit);

int main(int argc, char **argv) {

    char resolved_dict_buf[255];
    const char* dict_path = NULL;
	const char* script_path = NULL;
    const char* out_file = NULL;
    u1 bQuite = 0;
    int rs = 0;

    //Segmentor seg;
    ::google::SetUsageMessage("segment Chinese text.\n mmseg -dict_path <the_path> <file_to_process>\n");
    ::google::ParseCommandLineFlags(&argc, &argv, true);

    if(argc < 2) {
        ::google::ShowUsageWithFlags(argv[0]);
        return -1;
    }
    //printf("%d, %s\n",argc,argv[1]);
    //return 0;
    /*
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
    */
    out_file = argv[1]; // file to be process

    dict_path = FLAGS_dict_path.c_str();
	script_path = FLAGS_script_path.c_str();

    const char* resolved_dict_path = realpath(dict_path, resolved_dict_buf);
	std::string s_dict_path(resolved_dict_path);

	resolved_dict_path = realpath(script_path, resolved_dict_buf);
	std::string s_script_path(resolved_dict_path);

#if MMSEG_DEBUG
    printf("dict=%s; file=%s\n", s_dict_path.c_str(), out_file);
#endif


    // load basic diction only -> for system booting up.
    {
        int n = segment(out_file, s_dict_path.c_str(), s_script_path.c_str(), false);
    }
    return 0;
}

#if 1
int segment(const char* utf8_file, const char* dict_path, const char* script_path, u1 b_quit) {
    /*
     *  1 load all text from disk, file must encode in utf8
     *  2 strip BOM
     *  3 send it to seg, wait result.
     */
    int rs = 0;
    size_t length;
    char* buffer = NULL;
    char* buffer_ptr = NULL;
    const unsigned char txtHead[4] = {239,187,191};

    // load buffer
    {
        std::ifstream is(utf8_file);
        //load data.
        is.seekg (0, std::ios::end);
        length = (size_t)is.tellg();
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
        /*
         * 1 load term
         * 2 load pharse
         * 3 load special
         * ---------------
         * 4 load script
         * 5 init options
         * 6 do token
         */

        std::string s_dict_path(dict_path);
        mm::DictMgr mgr;
        mm::SegScript script_mgr;
        std::string s_error;
        // load script stage, if script execute wrang, no needs load dictionary any more
        // FIXME: deal with script engine later.
        int rs = script_mgr.LoadScripts( script_path, s_error);
        if(rs < 0) // the script loaded, if <0, error
        {
            LOG(ERROR) << "script load error. " <<  s_error;
        }

        if(1)
        {
            // load dict stage
            mgr.LoadTerm(dict_path);
            mgr.LoadPharse(dict_path);
            mgr.LoadSpecial(dict_path);

            std::string s_idx_cache = pystring::os::path::join(s_dict_path, ".term_idx");
            rs = mgr.LoadIndexCache(s_idx_cache.c_str());
            if( rs < 0) {
                // manual build idx, load cache failure.
                LOG(INFO) << "rebuild cache index " <<  s_idx_cache;
                mgr.BuildIndex(true);
                mgr.SaveIndexCache(s_idx_cache.c_str()); //might failure.
            }else {
                LOG(INFO) << "load cache index " <<  s_idx_cache;
                mgr.BuildIndex();
            }
        } // end load dict.

        mm::SegStatus* seg_stat = new mm::SegStatus();  // huge memory alloc, needs alloc on heap.
        mm::Segmentor seg(mgr, script_mgr);
        int task_id = 0;
        rs = seg.Tokenizer(task_id, buffer, length, seg_stat);
        while(rs > 0) {
            // should round by while,
            // dup the output.
            rs = seg.Tokenizer(task_id, NULL, 0, seg_stat); // state call
        }
        delete seg_stat; // clear
    }

    // free memory
    {
        delete[] buffer;
    }
    return 0;
}
#endif

/* end of file */
