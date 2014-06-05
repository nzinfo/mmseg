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


#if !defined(_SEGSCRIPT_H)
#define _SEGSCRIPT_H

#include "mm_segpolicy.h"
#include "mm_api_script.h"

namespace mm {

/*
 * 实际为 Script Manager， 从指定目录加载脚本文件
 *
 * - 暴露 API 给脚本
 * - 本对象全局唯一
 *
 *
 */
class SegScript {
public:
    SegScript() {
        _script = new LUAScript();
        lua_script_init(_script);
    }

    virtual ~SegScript();

public:
	//SegScriptPeer _peer;
    /*
     * load script from disk.
     * load order by ascii orer
     */
    int LoadScripts(const std::string script_path, std::string & s_err);

protected:
    LUAScript* _script;
};

} // namespace mm

//////////////////////////////////////////////////////////////////////////////////////////

#endif  //_SEGSCRIPT_H
