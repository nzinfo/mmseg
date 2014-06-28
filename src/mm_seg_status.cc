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

#include <iostream>
#include <fstream>
#include <glog/logging.h>

#include "utils/utf8_to_16.h"
#include "mm_seg_status.h"

namespace mm {

SegStatusSwapBlock::SegStatusSwapBlock(int block_size)
    :_size(block_size), _icode_last_s_pos(0)
{
    u4 size_of_icodes = (_size+4)*sizeof(UnicodeSegChar);   //额外冗余 4 个字符，用于 B1B2 E2E1
    _icodes = (UnicodeSegChar*)malloc(size_of_icodes); // include padding. B1B2; E2E1
    memset(_icodes, 0, size_of_icodes);

    _icode_chars = (u4*)malloc((_size+4)*sizeof(u4)); // 反正  match 已经开到1M了，这里额外用32K，大丈夫无所谓了。
    memset(_icode_chars, 0, (_size+4)*sizeof(u4));
    _icode_matches = (u4*)malloc((_size+4)*sizeof(u4)); // 用于记录在特定位置，都有多少候选词。存在技巧压缩，不折腾了。
    memset(_icode_matches, 0, (_size+4)*sizeof(u4));

    u4 size_of_matches = _size*sizeof(DictMatchEntry)*32; // 1M, less than one picture.
    _matches_data_ptr = (u1*)malloc( size_of_matches );
    memset(_matches_data_ptr, 0, size_of_matches);
    _matches = new DictMatchResult( _matches_data_ptr, _size*32 );
}

SegStatusSwapBlock::~SegStatusSwapBlock()
{
    free(_icodes);
    free(_icode_chars);
    free(_icode_matches);

    delete _matches;
    free(_matches_data_ptr);
}


int SegStatusSwapBlock::SetBuffer(const char* buf, u4 len){
    return 0;
}

void SegStatusSwapBlock::Reset()
{
    u4 size_of_icodes = (_size+4)*sizeof(UnicodeSegChar);
    memset(_icodes, 0, size_of_icodes);
    memset(_icode_chars, 0, (_size+4)*sizeof(u4));
    memset(_icode_matches, 0, (_size+4)*sizeof(u4));
    _matches->Reset();
}

SegStatus::SegStatus(SegOptions &option, u4 size) 
	:_options(option), _size(size)
{

    _icode_pos = 0;

    // 初始化 blocks
    _block1 = new SegStatusSwapBlock(size);
    _block2 = new SegStatusSwapBlock(size);
    _block_active = _block1;
}

SegStatus::~SegStatus() {
    _block_active = NULL;
    delete _block1;
    delete _block2;
}

void SegStatus::Reset() {

    _text_buffer_ptr = _text_buffer = NULL;
    _text_buffer_len = 0;

    _block_active = _block1;
    _block1->Reset();
    _block2->Reset();

    _icode_pos = 0;
    _icode_last_s_pos = 0;

}

int SegStatus::SetBuffer(const char* buf, u4 len) {
    _text_buffer_ptr = _text_buffer = buf; // 此处不保持对 buffer 的占有
    _text_buffer_len = len;

    _block1->SetBuffer(buf, len);
    _block2->SetBuffer(buf, len);


    /*
     * 使用 PUA 区域的 icode  { B1B2 ctx E2E1 }
     */
    ActiveBlock()->_icodes[_icode_pos].origin_code = SEG_PADING_B1;
    ActiveBlock()->_icodes[_icode_pos].tagA = SEG_PADING_TAGTYPE; // 127
    ActiveBlock()->_icodes[_icode_pos].tagB = 'S';
    ActiveBlock()->_icode_chars[_icode_pos] = SEG_PADING_B1;
    _icode_pos ++;
    ActiveBlock()->_icodes[_icode_pos].origin_code = SEG_PADING_B2;
    ActiveBlock()->_icodes[_icode_pos].tagA = SEG_PADING_TAGTYPE;
    ActiveBlock()->_icodes[_icode_pos].tagB = 'S';
    ActiveBlock()->_icode_chars[_icode_pos] = SEG_PADING_B2;
    _icode_pos++;

    return 0;
}

int SegStatus::MoveNext() {
    /*
     * 把 _icode 之类的状态，切换到下一个 block 可用
     *  - 因为还要保留 annote　等信息，因此无法使用　Reset， 实际上 Reset 不会被调用。
     *
     * 返回，当前 的 _icode_pos
     *
     */
    NextBlock()->Reset();

    LOG(INFO) << "MNext " <<  _icode_last_s_pos << "->" << _icode_pos;
    u4 icode_last_s_pos = _icode_last_s_pos;
    if(!_icode_last_s_pos)
        icode_last_s_pos = _icode_pos - 50; //如果小于 50 ? 则本函数根本不该调用
    if(_icode_pos - 50 < 0 )
        return -1; // verify avoid crash.

    // move remain char.
    if(icode_last_s_pos != _icode_pos-1) {
        memmove(NextBlock()->_icodes, (ActiveBlock()->_icodes) + icode_last_s_pos,
                sizeof(UnicodeSegChar)*(_icode_pos - icode_last_s_pos) );
        memmove(NextBlock()->_icode_chars, (ActiveBlock()->_icode_chars) + icode_last_s_pos,
                sizeof(u4)*(_icode_pos - icode_last_s_pos) );
        /*
         *  此处，前后两个 block 存在一部分数据的重合
         */
        ActiveBlock()->_icode_last_s_pos = icode_last_s_pos;

        // 不需要移动 _icode_matches 因为要重新找。
        _icode_pos = _icode_pos - icode_last_s_pos; // 新区段的 icode_pos
    }else{
		_icode_pos = 0;
	}
    icode_last_s_pos = 0;

    // 处理 AnnotePool 的轮换; 使用轮换的初衷是不希望在 block 切换时，丢失上下文的标引信息。
    SwapBlock();
    return _icode_pos;
}

const DictMatchResult* SegStatus::GetMatchesAt(u4 pos, u2* count) {
    /*
     * 得到某个 icode 位置上的全部候选的词。
     */
	return NULL;
}

u1 SegStatus::SetTagA(u4 pos, u1 tag) {
	return 0;
}

u1 SegStatus::SetTagB(u4 pos, u1 tag) {
	return 0;
}

u1 SegStatus::SetTagPush(u4 pos, u1 tag) {
	return 0;
}

void SegStatus::BuildTermIndex() {
    /*
     * 因为新的 RuleHit机制， TermIndex 不再重要。可以把需要检索的词，分配一类编号。检查这类编号即可。
     */
}

u4 SegStatus::FillWithICode(const DictMgr &dict_mgr, bool toLower) {
    /*
     * 从当前ptr的转换新的icode ，并进行步进
     * 从 dict_mgr 中查询 tolower & tag.
     *
     * 此步完成后 tagA 是字符的类型， tagB 是按照类型计算的切分信息。BMES
     *
     */
	if(0) // disabled test code.
    {
        // check icode with ucs32; --> decode is good.
        u1 buf[128];
        u2 rsn = 0;
        int n = csr::csrUTF8Encode(buf, 0x4F00000u);
        int rs = csr::csrUTF8Decode(buf, rsn);
        if(rs == 0x4F00000u)  {
            printf("decode ok.");
        }
        return 0;
    } // end if of check utf8 decode.

    u4 icode_pos_begin = _icode_pos;
    int icode = 0;
    u4  icode_lower = 0;
    u2  icode_tag = 0;

    u2 utf8_icode_len = 0;
	bool b_decode_error = false;
    mm::UnicodeSegChar* _icodes = ActiveBlock()->_icodes;
	u4*  _icode_chars = ActiveBlock()->_icode_chars;

    while( (_icode_pos < _size)
     && (_text_buffer_ptr < _text_buffer + _text_buffer_len)
    ) {
        icode = csr::csrUTF8Decode (
                    (const u1*)_text_buffer_ptr, utf8_icode_len);
		if(icode < 0) {
			/*  因为不能假设 输入是一个有效的 utf-8， 因此 此处 不能让系统崩溃。 但是从此以后的数据不再解码。 */
			b_decode_error = true;
            // give some ctx info. 肯定 break 了部分字符。但是足以给出上下文信息提示。
            //const char* text_buffer_ctx_ptr = (_text_buffer_ptr - _text_buffer) < 10 ? _text_buffer:_text_buffer_ptr-10;
            //LOG(ERROR) << "error decode utf-8 @pos " << (_text_buffer_ptr - _text_buffer) << text_buffer_ctx_ptr;
			break;
		}
		_text_buffer_ptr += utf8_icode_len; //move to next char.

		// look up via dict_mgr.
        icode_lower = dict_mgr.GetCharMapper()->Transform
                    ( (u4)icode, &icode_tag ); // the fact is only 11 byte of tag, currently we use only 8bit
        _icodes[_icode_pos].origin_code = icode;
        _icodes[_icode_pos].tagA = (u1)icode_tag;  // force case, change here if charmap's tag larger than 8bit.
        // 如果有小写的定义，则使用小写，否则，使用原始值。
        if(icode_lower)
            _icode_chars[_icode_pos] = icode_lower;
        else
            _icode_chars[_icode_pos] = icode;
        {
            // 计算 tagB 的值； 如果 与前一个的tagB 不同，则说明出现一个切分。
            if(_icodes[_icode_pos].tagA != _icodes[_icode_pos-1].tagA) {
                _icodes[_icode_pos].tagB = 'B';
                // fix prev B->S
                if(_icodes[_icode_pos-1].tagB == 'B') {
                    _icode_last_s_pos = _icode_pos-1;
                    _icodes[_icode_pos-1].tagB = 'S';
                }
                // fix prev M->E
                if(_icodes[_icode_pos-1].tagB == 'M')
                    _icodes[_icode_pos-1].tagB = 'E';
            }else{
                _icodes[_icode_pos].tagB = 'M';
            }
        }
		_icode_pos++;
    } // end while

    // check is reach the end of text
    if( b_decode_error  ||  ! (_text_buffer_ptr < _text_buffer + _text_buffer_len) ) {

		if(_icode_pos != icode_pos_begin) {  //else no char advance error @begin.
			// fix prev B->S
			if(_icodes[_icode_pos-1].tagB == 'B')
				_icodes[_icode_pos-1].tagB = 'S';
			// fix prev M->E
			if(_icodes[_icode_pos-1].tagB == 'M')
				_icodes[_icode_pos-1].tagB = 'E';

			// the end of buffer. 或者 解码出现错误。
			_icodes[_icode_pos].origin_code = SEG_PADING_E2;
			_icodes[_icode_pos].tagA = SEG_PADING_TAGTYPE;
			_icodes[_icode_pos].tagB = 'S';
			_icode_chars[_icode_pos] = SEG_PADING_E2;
			_icode_pos++;
			_icodes[_icode_pos].origin_code = SEG_PADING_E1;
			_icodes[_icode_pos].tagA = SEG_PADING_TAGTYPE;
			_icodes[_icode_pos].tagB = 'S';
			_icode_chars[_icode_pos] = SEG_PADING_E1;
			_icode_last_s_pos = _icode_pos;
			_icode_pos++;
            //LOG(INFO) << "Add Doc EndMark " << _icode_pos << " " << icode_pos_begin;
        }
    }
    return _icode_pos - icode_pos_begin; // how many char filled.
}

u4 SegStatus::BuildTermDAG (const DictMgr& dict_mgr, const DictTermUser *dict_user)
{
	/*
	 * 使用前缀搜索，构造 词网格
     * 1 检索全局词典
     * 2 检索 special 词典
     * 3 检索 用户词典
     * 4 对检索结果进行排序
     *
     * 与 Annote 有关
     * 1 根据 Option 与 dict_mgr 计算需要读取的属性信息
     *      dict_id, field_type, field_offset
     * 2
     *
	 */

	mm::DictBase* special_dict = NULL;
    if(_options.GetSpecialDictionaryName() != "")
        special_dict = dict_mgr.GetDictionary(_options.GetSpecialDictionaryName().c_str());

    int num = 0;
	DictMgr& mgr = (DictMgr&)dict_mgr;
    for(u4 i = 0; i < _icode_pos; i++) {
        // 从全局中检索，并不对读取到的值进行扩展 （ 1 不一定需要属性信息，如词频； 2 降低 _matches 的数量（长度相同没必要重复了） ）
        int rs = mgr.PrefixMatch(ActiveBlock()->_icode_chars + i, _icode_pos - i, ActiveBlock()->_matches, false); // matches will advence
        if(rs == -1)
            return -1; // should assert too many matches.
        if(i)
            ActiveBlock()->_icode_matches[i] = rs + ActiveBlock()->_icode_matches[i-1];
        else
            ActiveBlock()->_icode_matches[i] = rs;
        //printf("match @ %d, count %d.\n", i, _icode_matches[i]);
        num += rs;
    }
    return num;
}

int SegStatus::Apply(const DictMgr& dict_mgr, SegPolicy* policy)
{
    if(policy == NULL)
        return -1; // should crash.
    return policy->Apply(dict_mgr, *this);
}

int SegStatus::Annote(u4 pos, u2 token_len, u2 source_id, const char* prop_name,
           const u1* data_ptr, u4 data_len, bool bReplace)
{
    u2 prop_id = GetOption().GetAnnoteID(prop_name);
    return AnnoteByPropID(pos, token_len, source_id, prop_id, data_ptr, data_len, bReplace);
}

int SegStatus::AnnoteByPropID(u4 pos, u2 token_len, u2 source_id, u2 prop_id,
           const u1* data_ptr, u4 data_len, bool bReplace)
{
    /*
     * Annote 的存储格式；
     * 8 byte
     *  - 4 byte 存储 类型 | 偏移量 或 实际的值
     *  - 4 byte 存储在 Annote 数组中，同一个位置的 Annote 的下一条记录的 idx
     *
     * 在存储类型的 1 byte 中，如果最高位的bit 为 1 ，该位置存储的是实际的值（占后续 3byte），而非偏移量
     *      如最高位 为 0， 则后续的 3byte 为对应 stringpool 的偏移量 (最多 24M， 由于当前的Block长度为 ~ 8K，似乎足够)
     *
     * 类型的表示，占用 7个 bit，最多 127； 0 系统保留，用于 Annote 对应位置的 Term 的基本类型 （ 中文［包括日韩］ | 西文 | 数字 |　标点）
     *
     * 1 Annote 可以在运行时，由脚本增加，脚本增加的类型，不受 Option 指定的限制，如果该类型不存在，则自动增加
     * 2 写结果时，只返回 Option 指定的 Annote
     * 3 系统保留的（全切分等），由系统规则自动增加，不占用类型编号，也不可以被脚本查询
     * 4 Annote  基本的（AnnoteType=0）Annote，与 DAG 一一对应，可以利用对应的DAG的TermLen信息，得到要标引的文字长度
     * 5 位置 Offset（低4byte)中最高的1byte，保存Annote的来源，来自词典的，为词典编号；来自脚本的，取其给定值，必须 > 32;
     *
     */
    if(0) // debug the annote append.
    {
        u1 buf[128];
        int n = 0;
        for(u4 i = pos; i< pos+token_len; i++ ){
            n = csr::csrUTF8Encode(buf, ActiveBlock()->_icode_chars[i]);
            buf[n] = 0;
            fprintf(stdout, "%s", buf );
        }
        printf(" annote at %d: key=%s; v= %.*s \n", pos, GetOption().GetAnnoteName(prop_id), data_len, data_ptr);
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////
void SegStatus::_DebugCodeConvert()
{
	/* 调试使用的 iCode ， 输出 UTF8 字符串 & 对应的 icode， 用 ' ' 分割 */
	u1 buf[128];
	int n = 0;
	for(u4 i = 0; i< _icode_pos; i++ ){
        n = csr::csrUTF8Encode(buf, ActiveBlock()->_icode_chars[i]);
		buf[n] = 0;
        printf("%s(%lu->%lu) ", buf, ActiveBlock()->_icodes[i].origin_code, ActiveBlock()->_icode_chars[i]);
        if( ActiveBlock()->_icodes[i].tagB == 'E' || ActiveBlock()->_icodes[i].tagB == 'S' )
        {
            printf("/ ");
        }
	}
}

void SegStatus::_DebugDumpDAG()
{
  /*
   *  调试 DAG，  输出 字 ， 词的候选
   */
  u1 buf[128];
  int n = 0;

  int pos = 0;
  const DictMatchEntry* match_entry = NULL;
  for(u4 i = 2; i< _icode_pos; i++ ){
      n = csr::csrUTF8Encode(buf, ActiveBlock()->_icode_chars[i]);
      buf[n] = 0;
      fprintf(stdout, "%s ", buf );
      // 因为 B1 B2 不可能出现在词库中，因此此处 i 必然 > 0
      for(u2 j = 0; j < (u2)(ActiveBlock()->_icode_matches[i] - ActiveBlock()->_icode_matches[i-1] ); j++) {
          match_entry = ActiveBlock()->_matches->GetMatch(pos);
          if(match_entry) {
              fprintf(stdout, "d = %d, c = %d; ", match_entry->match._dict_id, match_entry->match._len );
          }
          pos ++;
      }
      fprintf(stdout, "\n");
  } // end for
}

void SegStatus::_DebugMMSegResult()
{
    /*
     *  输出 mmseg 的切分结果
     */
    u1 buf[128];
    int n = 0;

    int pos = 0;
    const DictMatchEntry* match_entry = NULL;
    for(u4 i = 0; i< _icode_pos; i++ ){
        n = csr::csrUTF8Encode(buf, ActiveBlock()->_icode_chars[i]);
        buf[n] = 0;
        printf("%s %c", buf , ActiveBlock()->_icodes[i].tagSegA);
        // 因为 B1 B2 不可能出现在词库中，因此此处 i 必然 > 0

        printf("\n");
    } // end for
}

} // namespace mm
