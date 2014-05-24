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

namespace mm {

typedef union AnnoteEntry {
	struct {
        u4 _offset;			//  在标引数据区的偏移量。
		u2 _len;			//  标引需要的长度
		u1 _type;			//	标引的类型，一个任务的最多 255 种 不同的标引类型
		u1 _source;			//  标引的来源， 从那个词典来的 0~31 词典； 31 以上， 由脚本定义
	} entry;
	u8 v;
}AnnoteEntry;				// 本质是一个 u8 在 64 bit 机器上可以一次执行完。 （字符的信息如何存？）

class AnnotePool {
    /*
     *  存储 Annote 的数据，通过 AnnoteEntry 的 _offset 指向
     *  类型，长度，数据。类型定义同 dictentry
     *  当 SegStatus MoveNext 后， AnnotePool 在特定的时间会被清空。
     */
};

class SegStatus {
	/*
	 * 执行分词使用的上下文。
	 */
public:
	SegStatus(u4 size);		// 一个处理批次可以处理的文字数量
    virtual ~SegStatus();
	void Reset();

public:
	// Similar with Reset, but slide window to next valid postions.
	int MoveNext();		// 不是移动到下一个字，而是移动到下一个处理批次的起始位置
	bool IsPause();		// 
	const DictMatchResult* GetMatchesAt(u4 pos, u2* count);
	// Called By SegPolicy
	u2 SetTagA(u4 pos, u2 tag);
	// Use by LUA Script, set the highest 8bit as tag.
	// if in CRFMode, tagA is the result of assistant tokenizer's result
	u2 SetTagB(u4 pos, u2 tag);
	// Almost the save as tagB, but Push the originB -> A, if originB is not 0;
	// Used by SegPolicy chain.
	u2 SetTagPush(u4 pos, u2 tag);
	// Build property's index. 用于支持 find term by property 
	void BuildTermIndex();

protected:
    DictMatchResult* _matches;  // 从词典中读取到的命中信息， 必须按照长度排序
    u8* _icodes;	// 当前正在处理的上下文， 如果处理完毕， 会更新, 保存 tag 和 实际的 icode
	u4 _offset;		// 从起点开始， 现在的偏移量
	u4 _size;		// 整个 status 的最大字符容量
    unordered_map<u4, AnnoteEntry> _annotes;    // offset -> annote 的表
    AnnotePool* _annote_pool1;                  // 存储 annote　的数据
    AnnotePool* _annote_pool2;
    AnnotePool* _annote_pool_active;  // a pointer of _annote_pool1 | _annote_pool2
	SegOptions * _options;
};

} // namespace mm

#endif  //_SEGSTATUS_H
