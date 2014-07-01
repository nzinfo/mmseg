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

#if !defined(_SEGSTATUS_H)
#define _SEGSTATUS_H

#include "mm_hashmap.h"
#include "mm_seg_option.h"
#include "mm_dict_mgr.h"
#include "mm_dict_term_user.h"
#include "mm_segpolicy.h"
#include "mm_stringpool.h"

// A temporary place store token result & intermedia data
// 
// the least 31 char will be been passed to next SegScript, due to SegWindow, Except it's the very end.
// 
// the least 15 token will not be poped out, due to SegWindow, Except it's the very end.
// 
// _matches: used to build termEntry's lattice , = 3* _size
// 
// _icodes: the iCode value of currrent status.
//     u4: real icode
//     u2: type     the chartype from charmapper.
//     u1: seg tag
//     u1: seg tag by script.
// 
// if annote's value is the same, only _source is diff, use the 1st one.
// 
// if match is full Status will notify Segmentor have a pause.
// if annote is full, 
// 
// What's happened when annote buffer is full when script process? 
// 1 a vector of annote will be use;
// 2 the vector will be reset after Reset | MoveNext
// 
// 
// Annote Pool have a swap mechanism. Once MoveNext called, 
// 1 set counter pool as active
// 2 clear the active
// 3 clear _annotes , where key < _offset
// 
// if _annotes have prev annote, it's in unactive part, and still accessible.
#define SEG_STATUS_DEFAULT_BATCH    4096    // 4K uni-char.

#define SEG_PADING_B1           0xF0000u
#define SEG_PADING_B2           0xF0002u
#define SEG_PADING_E2           0x10FFFEu
#define SEG_PADING_E1           0x10FFFFu
#define SEG_PADING_TAGTYPE      0x7Fu

namespace mm {

/*
 * 1 得到词典的全部属性的名字
 *      属性名 -> ( 词典编号 | 词典的字段 ）
 *
 * 2 检查是否是保留的名字
 *      - 创建词典时，也需要检查；
 *      - 目前包括 origin;stem;allterm
 *
 * 3 如果标引是词典的某个字符串类型的属性的值，直接引用对应的地址
 *
 * 4 如果不是，引用某个字符串Pool 的地址
 *
 * 5 标引的类型编号，由 Option 控制
 *
 * 6 标引对应的是字符的位置上， 输出词条时，检查这个词典占用的全部字符位置上的标引信息（只检测首字的？）
 *
 * 7 allterm 需要特殊处理。
 *
 * 8 通过脚本增加标引 -> 脚本的 API
 *
 */

typedef struct AnnoteEntry {
    u4 flag_value;          //  flag + type + value|offset_in_stringpool
    u4 source_next_idx;     //  source + next_element_in_array.
}AnnoteEntry;				//

typedef std::vector<AnnoteEntry> AnnoteEntryList;


class Segmentor;
class SegPolicyMMSeg;
class SegmentorResultReader;

typedef struct UnicodeSegChar {
    u4 origin_code;
    //u2 code;    // 转换为小写的　icode , 如果为 0 则取 origin_code. 移出，用于prefixmatch
    u1 tagA;      // 字符在 Unicode 标准中定义的 Script Type
    u1 tagB;      // 根据 ScriptType 的切分方案， 此处有冗余。
    u1 tagSegA;   // 辅助分词法 （MMSeg）
    u1 tagSegB;   // 最终的分词结果。 如果最终定下来后，在处理词性标注|NER之前，可以将数据存入 tagA 或 tagB
}UnicodeSegChar;

class SegStatusSwapBlock {
   /*
    * 一个 Status 包括两个 SwapBlock， 用于保障处理分词流时的上下文一致性。
    * 1 需要传递整个的 Buffer， 以便于 Block 可以自行　Verify 数据的一致性
    * 2 保存： iMatch Match Annote， 当Status切换时，清除上上个
    */
public:
    SegStatusSwapBlock(int block_size = SEG_STATUS_DEFAULT_BATCH);
    virtual ~SegStatusSwapBlock();

public:
    int     SetBuffer(const char* buf, u4 len);     // 此处设置的是整段的 buffer
    void    Reset();   //清除全部数据， 包括 SetBuffer 的
    void    SetiCodeEndMarker(u4 icode_last_s_pos) {
        /*
         * 设置本区段的最高有效偏移量， 因为部分数据与后续的Block 重复
         */
        _icode_last_s_pos = icode_last_s_pos;
    }

public:
    mm::DictMatchResult* _matches;          // 从词典中读取到的命中信息， 必须按照长度排序, 对应到当前的 Block
    mm::UnicodeSegChar*  _icodes;           // 当前正在处理的上下文， 如果处理完毕， 会更新, 保存 tag 和 实际的 icode
    u4*             _icode_chars;           // 保存unicode 的原始值 和 tolower 后的值（如果有），用于 prefixmatch.
    u4*             _icode_matches;         // 按照词的位置，给出都命中了多少词条。 处理为累计，使用 - 得到实际的数量
    AnnoteEntryList _annote_list;           // 本区块对应的 Annote
    mm::StringPoolMemory _annote_pool;      // annote 被处理为字符串.. so...

    u4               _icode_last_s_pos;     // 保存最后一个有效的 icode 位置

protected:
    u1*              _matches_data_ptr;     // 实际存 match 大的区域
    u4               _size;                 // 整个 status 的最大字符容量

};

class SegStatus {

    // 这里暗示了一个 设计缺陷， 但是我没想好怎么改。
    friend class Segmentor;
	friend class SegPolicy;
    friend class SegPolicyMMSeg;
    friend class SegmentorResultReader;

	/*
	 * 执行分词使用的上下文。
	 */
public:
    SegStatus(SegOptions& option, u4 size = SEG_STATUS_DEFAULT_BATCH);		// 一个处理批次可以处理的文字数量
    virtual ~SegStatus();
	void Reset();

public:
    int SetBuffer(const char* buf, u4 len);

	// Similar with Reset, but slide window to next valid postions.
	int MoveNext();		// 不是移动到下一个字，而是移动到下一个处理批次的起始位置
    bool HasMoreData() {
        return _text_buffer_ptr < _text_buffer + _text_buffer_len;
    }

    const mm::DictMatchResult* GetMatchesAt(u4 pos, u2* count);
	// Called By SegPolicy
    u1 SetTagA(u4 pos, u1 tag);
	// Use by LUA Script, set the highest 8bit as tag.
	// if in CRFMode, tagA is the result of assistant tokenizer's result
    u1 SetTagB(u4 pos, u1 tag);
	// Almost the save as tagB, but Push the originB -> A, if originB is not 0;
	// Used by SegPolicy chain.
    u1 SetTagPush(u4 pos, u1 tag);
	// Build property's index. 用于支持 find term by property 
	void BuildTermIndex();
    SegOptions& GetOption() { return _options; }

public:
    /*
     * 在位置 pos, 给 token_len 的 token 添加标引信息。
     *
     * 标引类型为  prop_name; 数据为 data_ptr
     *
     * 设置标记为 bReplace, 如果为 True，则在同样的位置、同样的长度，只能有一个同名的标引
     *
     * u2 source_id 表示属于哪个词典 或 脚本给出的 tag
     *
     */
    int Annote(u4 pos, u2 token_len, u2 source_id, const char* prop_name,
               const u1* data_ptr, u4 data_len, bool bReplace = false);

    int AnnoteByPropID(u4 pos, u2 token_len, u2 source_id, u2 prop_id,
               const u1* data_ptr, u4 data_len, bool bReplace = false);

    int AnnoteTermType(u4 dag_pos, char term_type);

protected:
    // Segmenter's Intractive functions.
    u4 FillWithICode(const DictMgr& dict_mgr, bool toLower = true); // 转换到 icode, 转换到小写（常用字）
	// 根据 词典生成候选词表, 返回 DAG 图中的元素个数。可以同时加载 用户自定义词库 与 专用的一个领域词库。
    u4 BuildTermDAG (const DictMgr& dict_mgr, const DictTermUser *dict_user = NULL);

    int Apply(const DictMgr& dict_mgr, SegPolicy* policy);   //不使用 const，因为有些policy 可能有上下文词典，需要修改自身。（虽然理论上不应）

protected:
	void _DebugCodeConvert();
    void _DebugDumpDAG();
    void _DebugMMSegResult();

protected:
    inline SegStatusSwapBlock* ActiveBlock() {
        return _block_active;
    }

    inline SegStatusSwapBlock* NextBlock() {
        if(_block1 == _block_active)
            return _block2;
        return _block1;
    }

    inline void SwapBlock() {
        if(_block1 == _block_active) {
            _block2->Reset();
            _block_active = _block2;
        }else{
            _block1->Reset();
            _block_active = _block1;
        }
    }

protected:

    u4 _icode_pos;  // 当前的位置，当切换时会被重置
    u4 _icode_last_s_pos; // 最后一个 根据 unicode script 标注为 S 的字的位置；需要检查如果为0， 则 _icode_last_s_pos == _icode_pos
    u4 _offset;		// 从起点开始， 现在的偏移量 （目前好像没用到）
	u4 _size;		// 整个 status 的最大字符容量

    SegStatusSwapBlock* _block1;
    SegStatusSwapBlock* _block2;
    SegStatusSwapBlock* _block_active;

    mm::SegOptions& _options;

protected:
    const char* _text_buffer;
    const char* _text_buffer_ptr;
    u4          _text_buffer_len;
};

} // namespace mm

#endif  //_SEGSTATUS_H
