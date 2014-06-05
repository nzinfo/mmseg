/*
 * Copyright 2014 Li Monan <limn@coreseek.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 */

#include <glog/logging.h>
#include <sstream>
#include <algorithm>

#include "mm_seg_script.h"
#include "utils/pystring.h"
#include "mm_dict_mgr.h"

namespace mm {

SegScript::~SegScript() {
    lua_script_clear(_script);

    delete _script;
}

int SegScript::LoadScripts(const std::string script_path, std::string &s_err)
{
    /*
     *  1 get all file in the path
     *  2 order it in asc
     *  3 load
     */
    std::vector<std::string> lua_scripts;
    // if path have more dict. only the first 20 , 24 solt, 0~3 reversed.
    int nfiles = mm::DictMgr::GetDictFileNames(script_path.c_str(), ".lua", true, lua_scripts);
    // order
    std::sort(lua_scripts.begin(),lua_scripts.end());
    // load each
    int rs = 0;
	std::string script_fname;
    for(std::vector<std::string>::iterator it = lua_scripts.begin();
         it != lua_scripts.end(); ++it) {
#ifndef WIN32
			 script_fname = pystring::os::path::join(script_path, *it);
#else
			 script_fname = pystring::os::path::join_nt(script_path, *it);
#endif // !WIN32
        rs = init_script(_script,  script_fname.c_str());  //FIXME: how to report error ?
        if(rs < 0) {
            // FIXME: addtional error from _script
            s_err = "error execute " + *it;
            return rs;
        }
        LOG(INFO) << "load script " << *it;
    }
    return nfiles;
}

} // namespace mm



