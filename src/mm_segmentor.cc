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
#include "mm_segmentor.h"
#include "mm_seg_status.h"
#include "utils/utf8_to_16.h"

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
        _mmseg.BindAnnote(_dict_mgr, *status); // check return value?
    }else {
		// move to next block.
		status->MoveNext();
	}

	int ichar_count = status->FillWithICode(_dict_mgr, true); // enable to lower case.
	if(ichar_count) {
		// FIXME： give a debug macro.
		//status->_DebugCodeConvert();
		int iterm_count = status->BuildTermDAG(_dict_mgr);		  // 使用 dictionary 构造对应的词网格（DAG）， 返回全部候选词的数量
		//在启用 人名库 | 组织名库后， 在此处即进行处理。 可能需要 CRF 对人名做二次确认。类似 Adaboost ?
		status->BuildTermIndex();                                 // 用于支持脚本对词条的快速查找
        //status->_DebugDumpDAG();
		int icode_seg = status->Apply(_dict_mgr, &_mmseg);		  // 应用具体的切分算法。 返回处理的文字个数
        //status->_DebugMMSegResult();
		//check is enable crfseg.
		//check enable pos
		//check enable ner.
	}

    //check is all data done.
    if(status->HasMoreData())
        return ichar_count; // 如果有数据，但是已经不能产生新的icode ，一样是结束。
    else
        return 0;
}

Segmentor::Segmentor(const DictMgr& dict_mgr, const SegScript& script_mgr, const DictTermUser *dict_user)
    :_dict_mgr(dict_mgr), _script_mgr(script_mgr)
{
    _dict_user = dict_user;

    // build the char's freq.
    _mmseg.BuildUSC2CharFreqMap(dict_mgr);
    // 构造读取 dict_mgr 中属性的列表

}

//////////////////////////////////////////////////////////////////
int SegmentorResultReaderFile::Feed(SegStatus* status)
{
    // 输出到 _fd
    u1 buf[128];
    int n = 0;

	UnicodeSegChar* _icodes = get_seg_char(status);
	u4*		   _icode_chars = get_icodes(status);

    int pos = 0;
    const DictMatchEntry* match_entry = NULL;
    for(u4 i = 0; i< icode_last_pos(status); i++ ){
        if(_icode_chars[i] >= SEG_PADING_B1)
            continue; // 自己增加的特殊字符。
        n = csr::csrUTF8Encode(buf, _icode_chars[i]);
        buf[n] = 0;
        fprintf(_fd, "%s", buf);
        if(_icode_chars[i] == '\r' || _icode_chars[i]=='\n' || _icode_chars[i] == '\t' || _icode_chars[i] == ' ') {
            continue; //已经输出了，就不要再额外给出 /S 了
        }
        // FIXME: 输出/x , 老传统了。 此外，应该输出 Annote， 目前没有 Annote 的支持。
        if(_icodes[i].tagSegA == 'S' || _icodes[i].tagSegA == 'E')
            fprintf(_fd, "/x ");
    } // end for
    return 0;
}

int SegmentorResultReaderScript::Feed(SegStatus* status)
{
    /*
     * 需要提供 可以输出切分 和 Annote 的能力， 标引不一定在 Term 的开头
     */
    _icodes = get_seg_char(status);
    _icode_chars = get_icodes(status);
    _icode_lastpos = icode_last_pos(status);
    return 0;
}

const char SegmentorResultReaderScript::Char(int pos, u2* annote_count_at_pos)
{
    if(pos >= 0 &&  _icode_lastpos - pos > 0) {
      if(_icode_chars[pos] >= SEG_PADING_B1)
        return 0; // 自己增加的特殊字符。

      if(annote_count_at_pos) {
          *annote_count_at_pos = 0;
      }
      return _icodes[pos].tagSegA;
    }
    // out of range.
    return 0;
}


} //end namespace mm.
