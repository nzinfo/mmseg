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

#include "mm_dict_base.h"
#include "mm_api_script.h"

/*
 *  用于测试脚本框架
 * 
 */

int
main(int argc, char* argv[])
{
    //mm::DictBase dict;

	LUAScript ctx;
	
	lua_script_init(&ctx);
	
	{
		const char* fname = argv[1];
		int n = init_script(&ctx, fname);
	}
	
	lua_script_clear(&ctx);
    return 0;
}
