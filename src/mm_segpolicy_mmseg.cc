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

#include "mm_segpolicy_mmseg.h"
#include "utils/pystring.h"
#include "mm_dict_mgr.h"
#include "mm_seg_status.h"

namespace mm {

typedef struct MMSegChunk {
   u4 term1_pos;  //存储的是词条的右侧的位置
   u4 term2_pos;
   u4 term3_pos;

   float avg(u4 iBegin) {
       /*
        * 计算词长度与平均词长度的方差
        */
       float avg = (float)1.0*(term3_pos-term3_pos)/3;  // token.size() == 3
       float total = 0;

       float diff = ((term1_pos - iBegin) - avg);
       total += diff*diff;

       diff = ((term2_pos - term1_pos) - avg);
       total += diff*diff;

       diff = ((term3_pos - term2_pos) - avg);
       total += diff*diff;

       return total;
   }

   float free(u4 iBegin, u4* iCodes_ptr, float* ucs2_freq_log) {
       /*
        * 计算根据词频的自由度， 在 mmseg 中，只有单字才存在自由度的概念。
        */
       float free_score = 0.0;
       if((term1_pos - iBegin) == 1 && iCodes_ptr[iBegin] < 65536)
           free_score += ucs2_freq_log[iCodes_ptr[iBegin]];

       if((term2_pos - term1_pos) == 1 && iCodes_ptr[term1_pos] < 65536)
           free_score += ucs2_freq_log[iCodes_ptr[term1_pos]];

       if((term3_pos - term2_pos) == 1 && iCodes_ptr[term2_pos] < 65536)
           free_score += ucs2_freq_log[iCodes_ptr[term2_pos]];
       return free_score;
   }

}MMSegChunk;

int SegPolicyMMSeg::Apply(const DictMgr& dict_mgr, SegStatus& status)
{
    // apply mmseg's rules
    /*
     * mmseg 算法的执行过程， 其中生成 ngram 的部分，可以复用 （实际未如此实现）
     * 0 如果 unicode scrip tag 基本认为字符是 单字，那么就是单字；如果不是 CJK 区的，直接输出 this->_cjk_chartag
     * 1 读取 3 个 term， 选出最长的长度的短语集合。
     * [删除] 2 计算 avl_length ， 平均长度, 因为有同样总长度的词有同样的平均长度，本规则实际不实现（ 因为有后缀 E2E1 )
     * 3 计算 min( avg ) , 各token长度与平均长度的方差，因为 token_count == 3, 所以，不需要再进行平均。
     *   （当未遇到后缀，此时算法工作不正常，需要轮换到下一个 Block 处理）
     * 4 计算 max freedom
     *   去词频算 log 的和，计算自由度需要额外的计算资源
     *
     * 步骤：
     * 1 构造词与词之间的bigram;
     * 2 计算 字 与 bigram 联合构成的 chunk 的 长度（取最大长度 & 长度与平均长度之方差 ）， 即同时处理 R1 & R3
     * 3 读取 单一字的自由度 （可以考虑在初始化的时候完成， 一个 64K 的 float, 直接存 log ）
     */
    int pos = 0;
    const DictMatchEntry* match_entry = NULL;
	const DictMatchEntry* match_entry2 = NULL;
    const DictMatchEntry* match_entry3 = NULL;

	int chunk_max_len = 0;
	int chunk_len = 0;
    float min_avg = FLT_MAX ;  //
	float max_freedom = 0.0; // 最大单字自由度， 只有单字才有意义。
	u4 i_level2 = 0;
	u4 i_level3 = 0;
	MMSegChunk current_chunk;
	MMSegChunk best_chunk;

	//for(u4 i = 0;  ; i++ ) 
	u4 i = 0;
    while(i< status._icode_pos -2 ) //最后 2 个 不是被截断的文字，就是E2E1; 需要回溯到上一个 tagB ， 才能移动数据。
	{	
        if(status._icodes[i].tagA != _cjk_chartag) {
            status._icodes[i].tagSegA = status._icodes[i].tagB;
            i++; continue; // 无视不是中文的。
        }
        if(status._icodes[i].tagB == 'S' ) {
            status._icodes[i].tagSegA = 'S';
            i++; continue; // 已经被标记为单字的，在 MMSeg 阶段继续保持（在 CRF 阶段则不同，因为需要进行新词发现）
        }
		if(status._icode_matches[i] == 0) {
			// 词库里一条记录都没有的家伙， 单着去。
			status._icodes[i].tagSegA = 'S';
			i++; continue; 
		}

        // 处理为三层循环，会有额外的计算量。从长到短， 贪心算优化; 晕， 盗梦空间
		// 也许不节省内存，计算 bigram 更好。取决于 循环执行的速度快还是内存访问的速度快。
        for(int term1_i = (status._icode_matches[i] - status._icode_matches[i-1]); term1_i >= 0; term1_i--) {
			//term1_i -= 1; //make it as an index
            if(term1_i) {
                pos = status._icode_matches[i-1] + term1_i - 1;  // status._icode_matches[i-1] is the begin position of current char stored.
                match_entry = status._matches->GetMatch(pos);
                // should never be NULL;
                i_level2 = i + match_entry->match._len; // advance to i + match_entry->match._len
            }else
                i_level2 = i + 1;

            current_chunk.term1_pos = i_level2;
            {
                // 规则：用户自定义词典优先，如果是用户自定义词 ( dict_id != 0 ), 则直接返回
               if(match_entry->match._dict_id)
                   break; // 直接转到 ·标注字符·
            }
			
			if(i_level2 >= status._icode_pos) // 因为 status._icode_pos -2 , 所以，必然存在最后两个字都是单字的路径。
				continue;

            for(int term2_i = (status._icode_matches[i_level2] - status._icode_matches[i_level2-1]); term2_i >= 0; term2_i--) {
				// 第二层循环
				//term2_i -= 1; //make it as an index
                if(term2_i) {
                    pos = status._icode_matches[i_level2-1] + term2_i - 1;
                    match_entry2 = status._matches->GetMatch(pos);
                    i_level3 = i_level2 + match_entry2->match._len; // advence to term3.
                }else
                    i_level3 = i_level2 + 1;

				current_chunk.term2_pos = i_level3;
				
				if(i_level3 >= status._icode_pos)
					continue;

                for(int term3_i = (status._icode_matches[i_level3] - status._icode_matches[i_level3-1]); term3_i >= 0; term3_i--) {
					//term3_i -= 1; //make it as an index
                    if(term3_i) {
                        pos = status._icode_matches[i_level3-1] + term3_i -1;
                        match_entry3= status._matches->GetMatch(pos);
                        // check len i_level3 - i + len
                        current_chunk.term3_pos = i_level3 + match_entry3->match._len;
                    }else
                        current_chunk.term3_pos = i_level3 + 1;

					chunk_len = current_chunk.term3_pos - i ;
					if(chunk_max_len < chunk_len) {
						// R1: 更新最长的 chunk
						chunk_max_len = chunk_len;
						best_chunk = current_chunk;
                    }else
					if(chunk_len == chunk_max_len) {
						// 应用 规则 3 4 确定合适的 chunk.
                        // R3: 最小的平均长度的方差
                        float avg = current_chunk.avg(i);
                        if(avg < min_avg) {
                            min_avg = avg;
                            best_chunk = current_chunk;
                        }else
                        if(abs(avg - min_avg) < 1E-6) {  //应该是  avg == min_avg, 最好不要依赖编译器判断 float 是否相同
                            // R4: 最大自由度
                            float free_score = current_chunk.free(i, status._icode_chars, _ucs2_freq_log);
                            if(max_freedom<free_score) {
                                max_freedom = free_score;
                                best_chunk = current_chunk;
                            }
                        }
					}else{
						// 长度不达标 ，无视
					}
				} // end for term3.
			} //end for term2
        } // end for term1
		
		// 标注字符
		if(best_chunk.term1_pos == i+1) {
			status._icodes[i].tagSegA = 'S';
		}else{
			status._icodes[i].tagSegA = 'B';
			for(u4 j = i+1; j<best_chunk.term1_pos-1;j++)
			{
				status._icodes[j].tagSegA = 'M';
			} // end for
			status._icodes[best_chunk.term1_pos-1].tagSegA = 'E';
		}// end if 
		i = best_chunk.term1_pos; //step to next term.
		// clear chunk status
		{
			chunk_max_len = 0;
			min_avg = FLT_MAX ;  
			max_freedom = 0.0; 
		}
    } // end for char
    return status._icode_pos;
}


int SegPolicyMMSeg::BuildUSC2CharFreqMap(const DictMgr &dict_mgr)
{
    /*
     * mmseg 需要词频的信息， 存在两种方案
     * 1 通过词库的唯一名称，得到 dict_id， 根据 dict_id & prop_id 进行读取；
     * 2 通过 prop_id 进行读取
     * 目前采用 方案1， 也就是 mmseg 的词库是必须的。
     * 基础词典的名称  com.coreseek.mmseg.base
     *
     */
    // 用于提前构造词频的自由度信息， 以减少二次读取的性能损失。因为是考虑字的自由度，对于 UTF-8大字集区的字不支持。
    // 查询的时候也应该额外检查。 iCode < 65535
    int mmseg_base_dict_id = dict_mgr.GetDictionaryID("com.coreseek.mmseg.base");
    CHECK_GT(mmseg_base_dict_id, 0) << "dictionary with name `com.coreseek.mmseg.base` not found!";

    DictBase* mmseg_base_dict = dict_mgr.GetDictionary("com.coreseek.mmseg.base");
    const mm::DictSchema* schema = mmseg_base_dict->GetSchema();
    // check prop_id
    const DictSchemaColumn* column = schema->GetColumn("freq");
    CHECK_NE(column, (const DictSchemaColumn*)NULL) << "dictionary  `com.coreseek.mmseg.base` have no field `freq`!";
    int column_id = column->GetIndex(); // the property's index in entrydata.

    int offset = 0;
    mm::EntryData* entry = NULL;

    for(u4 iCode = 0; iCode < 65536; iCode++) {
       // treated as only 1 freq.
       _ucs2_freq_log[iCode] = (float)log(1)*64; // in mmseg1, *100; I think *(2^x) might faster. log(1) == 0
       offset = mmseg_base_dict->ExactMatch(&iCode, 1);
       if(offset >=0 ) {
           entry = mmseg_base_dict->GetEntryDataByOffset(offset);
           if(entry) {
                _ucs2_freq_log[iCode] = (float)log( entry->GetU4(schema, column_id) + 1 ) * 64;
           }
       } // end if offset
    } // end for

    // 副作用， 确定 CJK 区的 Tag
    {
        u2 icode_tag = 0;
        dict_mgr.GetCharMapper()->Transform( (u4)0x4E2D, &icode_tag );  // Chinese Char `中`
        _cjk_chartag = icode_tag;
    }
	return 0;
}

} // namespace mm



