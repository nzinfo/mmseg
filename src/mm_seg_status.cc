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

SegStatus::SegStatus(SegOptions &option, u4 size) 
	:_options(option), _size(size)
{
    u4 size_of_icodes = 2*(_size+4)*sizeof(UnicodeSegChar);
    _icodes = (UnicodeSegChar*)malloc(size_of_icodes); // include padding. B1B2; E2E1
    memset(_icodes, 0, size_of_icodes);
    _icode_chars = (u4*)malloc(2*(_size+4)*sizeof(u4)); // 反正  match 已经开到1M了，这里额外用32K，大丈夫无所谓了。
    memset(_icode_chars, 0, 2*(_size+4)*sizeof(u4));
    _icode_matches = (u4*)malloc(2*(_size+4)*sizeof(u4)); // 用于记录在特定位置，都有多少候选词。存在技巧压缩，不折腾了。
    memset(_icode_matches, 0, 2*(_size+4)*sizeof(u4));
    _icode_pos = 0;

    u4 size_of_matches = size*sizeof(DictMatchEntry)*32; // 1M, less than one picture.
    _matches_data_ptr = (u1*)malloc( size_of_matches );
    memset(_matches_data_ptr, 0, size_of_matches);

    _matches = new DictMatchResult( _matches_data_ptr, size*32 );

    // 初始化 标引池
    //_annote_pool1 = new AnnotePool();
    //_annote_pool2 = new AnnotePool();
    //_annote_pool_active = _annote_pool1;
}

SegStatus::~SegStatus() {
    free(_icodes);
    free(_icode_chars);
    free(_icode_matches);
    delete _matches;
    free(_matches_data_ptr);

    //_annote_pool_active = NULL;
    //delete _annote_pool1;
    //delete _annote_pool2;

}

void SegStatus::Reset() {

    _text_buffer_ptr = _text_buffer = NULL;
    _text_buffer_len = 0;

	u4 size_of_icodes = 2*(_size+4)*sizeof(UnicodeSegChar);
    memset(_icodes, 0, size_of_icodes);
    memset(_icode_chars, 0, 2*(_size+4)*sizeof(u4));
    memset(_icode_matches, 0, 2*(_size+4)*sizeof(u4));
    _icode_pos = 0;
    _icode_last_s_pos = 0;
    _matches->Reset();

    // 处理标引
    //_annote_pool_active = _annote_pool1;
    //_annote_pool1->Reset();
    //_annote_pool2->Reset();
}

int SegStatus::SetBuffer(const char* buf, u4 len) {
    _text_buffer_ptr = _text_buffer = buf; // 此处不保持对 buffer 的占有
    _text_buffer_len = len;

    /*
     * 使用 PUA 区域的 icode  { B1B2 ctx E2E1 }
     */
    _icodes[_icode_pos].origin_code = SEG_PADING_B1;
    _icodes[_icode_pos].tagA = SEG_PADING_TAGTYPE; // 127
    _icodes[_icode_pos].tagB = 'S';
    _icode_chars[_icode_pos] = SEG_PADING_B1;
    _icode_pos ++;
    _icodes[_icode_pos].origin_code = SEG_PADING_B2;
    _icodes[_icode_pos].tagA = SEG_PADING_TAGTYPE;
    _icodes[_icode_pos].tagB = 'S';
    _icode_chars[_icode_pos] = SEG_PADING_B2;
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
    //LOG(INFO) << "MNext " <<  _icode_last_s_pos << "->" << _icode_pos;
    u4 icode_last_s_pos = _icode_last_s_pos;
    if(!_icode_last_s_pos)
        icode_last_s_pos = _icode_pos - 50;

    // move remain char.
    if(icode_last_s_pos != _icode_pos-1) {
        memmove(_icodes, _icodes+icode_last_s_pos, sizeof(UnicodeSegChar)*(_icode_pos - icode_last_s_pos) );
        memmove(_icode_chars, _icode_chars+icode_last_s_pos, sizeof(u4)*(_icode_pos - icode_last_s_pos) );
        // 不需要移动 _icode_matches 因为要重新找。
        _icode_pos = _icode_pos - icode_last_s_pos;
    }else{
		_icode_pos = 0;
	}
    memset(_icode_matches, 0, 2*(_size+4)*sizeof(u4));
    _matches->Reset();

    icode_last_s_pos = 0;

    // 处理 AnnotePool 的轮换; 使用轮换的初衷是不希望在 block 切换时，丢失上下文的标引信息。
    /*
    {
        if(_annote_pool_active == _annote_pool1) {
            _annote_pool2->Reset();
            _annote_pool_active = _annote_pool2;
        }else
        if(_annote_pool_active == _annote_pool2) {
            _annote_pool1->Reset();
            _annote_pool_active = _annote_pool1;
        }
    }
    */
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

    while( (_icode_pos < _size*2)
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
        int rs = mgr.PrefixMatch(_icode_chars + i, _icode_pos - i, _matches, false); // matches will advence
        if(rs == -1)
            return -1; // should assert too many matches.
        if(i)
            _icode_matches[i] = rs + _icode_matches[i-1];
        else
            _icode_matches[i] = rs;
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
    if(0) // debug the annote append.
    {
        u1 buf[128];
        int n = 0;
        for(u4 i = pos; i< pos+token_len; i++ ){
            n = csr::csrUTF8Encode(buf, _icode_chars[i]);
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
		n = csr::csrUTF8Encode(buf, _icode_chars[i]);
		buf[n] = 0;
		printf("%s(%lu->%lu) ", buf, _icodes[i].origin_code, _icode_chars[i]);
        if( _icodes[i].tagB == 'E' || _icodes[i].tagB == 'S' )
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
      n = csr::csrUTF8Encode(buf, _icode_chars[i]);
      buf[n] = 0;
      fprintf(stdout, "%s ", buf );
      // 因为 B1 B2 不可能出现在词库中，因此此处 i 必然 > 0
      for(u2 j = 0; j < (u2)(_icode_matches[i] - _icode_matches[i-1] ); j++) {
          match_entry = _matches->GetMatch(pos);
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
        n = csr::csrUTF8Encode(buf, _icode_chars[i]);
        buf[n] = 0;
        printf("%s %c", buf , _icodes[i].tagSegA);
        // 因为 B1 B2 不可能出现在词库中，因此此处 i 必然 > 0

        printf("\n");
    } // end for
}

} // namespace mm
