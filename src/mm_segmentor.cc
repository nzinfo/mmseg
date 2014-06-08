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


#include "mm_segmentor.h"
#include "mm_seg_status.h"

namespace mm {

int Segmentor::Tokenizer(u8 task_id, const char* text_to_seg, u4 text_len, SegStatus* status)
{
    /*
     *  实际进行中文切分, 如果还需要继续处理， 返回  > 0;
     *  如果出现错误 < 0
     *  数据全部处理完毕 0
     *
     *  1 初始化 status
     *  2 查询/设置 当前区段的 iCode
     *     - 需要预设 初始值（B1 B2 ） & 结尾 （E1 E2 )
     *     - 需要设置 原始的 icode & 转为小写 (common形式) 的 iCode
     *     - 提供调试的输出接口
     *  3 从词库中检索候选词，增加到词网格中。
     *  4 检查是否达到了 SegStatus 快满的状态
     *
     *  5 轮换的规则
     *    - Status 为双缓冲区，如当前处理的指针已经越过 Bounder, 则暂停。
     *    - 下一次调用时，自动将缓冲区上提 ？
     */
    if(text_to_seg) {
        status->Reset();
        status->SetBuffer(text_to_seg, text_len);
    }else {
		// move to next block.
		status->MoveNext();
	}

	int ichar_count = status->FillWithICode(_dict_mgr, true); // enable to lower case.
    // FIXME： give a debug macro.
    //status->_DebugCodeConvert();
	int iterm_count = status->BuildTermDAG(_dict_mgr);		  // 使用 dictionary 构造对应的词网格（DAG）， 返回全部候选词的数量
    status->BuildTermIndex();                                 // 用于支持脚本对词条的快速查找
    //status->_DebugDumpDAG();
    status->Apply(_dict_mgr, &_mmseg);				// 应用具体的切分算法。返回当前处理到的 text_to_seg_ptr 到 text_to_seg 的偏移量
    //check is enable crfseg.
    status->_DebugMMSegResult();
    //check enable pos
    //check enable ner.
    return 0;
}

Segmentor::Segmentor(const DictMgr& dict_mgr, const SegScript& script_mgr, const DictTermUser *dict_user)
    :_dict_mgr(dict_mgr), _script_mgr(script_mgr)
{
    _dict_user = dict_user;

    // build the char's freq.
    _mmseg.BuildUSC2CharFreqMap(dict_mgr);
}

} //end namespace mm.
