#ifndef SEG_RESULT_H
#define SEG_RESULT_H

/*
 * 中文分词的结果的定义
 *
 * 启动分词的选项
 *      1 是否允许 nbest
 *      2 是否允许 script
 *      3 是否进行词性标注
 *      4 允许使用那些 标引 (Annote)
 *      5 使用那些词典, 系统最多可以使用 32 个词典
 *      6 任务编号
 *      7 是否计时
 *
 * 使用 C 风格的接口 (因为需要额外的支持 Nginx, 因此不能使用 C++ ) , 定义检索结果的生成与解析
 *  包括:
 *    - meta 区, 用于记录本次分词的输入字符串长度
 *      * 本次分词的输入字符串长度
 *      * 本次的任务编号 64bit
 *      * 分词分词的原始字符串信息的 CRC32, 用于调用方校验数据
 *      * 包括那些数据:
 *          结果补丁区的记录数量|大小|增加的 token 数量
 *          是否包括词性信息
 *          包括那些 标引, 数据类型 | 名称 | 标引类型 (AnnoteTypeID 用于在标引区区分具体的标引数据)
 *          总共的切分出的 Token 数量
 *
 *    - 分词结果区
 *      使用 bitmap 标记那些 iCode 字符前面应该被切开;
 *      = 分词结果补丁区
 *      当出现多个标引结果时,记录某个 ( begin, end ) of iCode 的另外的切分方案; 可能存在多个切分补丁
 *      使用顺序编号引用切分的结果, 对于补丁区, 按顺序递增
 *    - 词性标注区
 *      按照 Token 编号的顺序, 标记 Token 的词性, 对于包括补丁区 的切分结果, 词性标注区顺序递增
 *    - 标引区
 *      ID, TokenID, AnnoteTypeID, AnnoteDataPtr    其中 ID 是基于偏移量的隐藏属性
 *          AnnoteDataPool
 *          为了便于处理, 一段不特定长度的 Annote 后, 会对TermID进行校准; 之后的记录 记录相对校准 TermID 后端的偏移量 ?
 *      对于使用 Pharse 切分方案的情况, 在 Annote 部分, 可以显示起 Pharse
 *      类型为 SubPharse 的 Annote, TermID = TermID_Begin, Data = TermIDEnd
 *
 */
#include "csr_typedefs.h"

typedef struct segresult_annote_def_
{
    char annote_data_type;  // u2 u4 u8 s
    char annote_name[8];    // 标引名称, 缩写; 标引名称的缩写为全局唯一.
}segresult_annote_def;

typedef struct segresult_annote_
{
    u4   token_id;
    u2   annote_id;                    // offset of annote_define
    u2   annote_data_size;
    u4   annote_data_offset;           // 数据存储在 data_pool 的偏移量
}segresult_annote;

typedef struct segresult_token_subseq_
{
    u4  text_icode_start;
    u4  subseq_icode_length;
    u8* token_bitmap;
}segresult_token_subseq;

typedef struct segresult_token_subseq_list_
{
    u4   count;                                     // 包括多少条标注信息
    segresult_token_subseq* seq;                    // 标注的数据
    struct segresult_token_subseq_list_* _next;     // 下一个记录标注的区域
}segresult_token_subseq_list;

typedef struct segresult_pos_list_
{
    u4   count;                                 // 包括多少条词性信息
    u2*  part_of_speech;                        // 词性列表
    struct segresult_pos_list_* _next;          // 下一个记录标注信息的区域
}segresult_pos_list;

typedef struct segresult_annote_list_
{
    u4   count;                                 // 包括多少条标注信息
    segresult_annote* note;                     // 标注的数据
    struct segresult_annote_list_* _next;       // 下一个记录标注的区域
}segresult_annote_list;

typedef struct segresult_annote_pool_list_
{
    u4   count;                                 // 数据的总长度
    u4   used;                                  // 已经分配出的长度
    u1*  data;                                  // 数据的实际地址
    struct segresult_annote_pool_list_* _next;  // 下一个记录词性信息的区域
}segresult_annote_pool_list;


typedef struct segresult_
{
    u8   task_id;                         //  任务编号, 由分词任务直接传入, 初始化值 为 200
    u4   text_icode_length;               //  传入的待切分的字符串的 iCode 长度
    u4   text_crc32;                      //  传入数据的 CRC32, 可选, 可以为 0
    u8   time_cost;                       //  切分的总计用时, 可选, 可以为 0

    u2   annote_def_count;                //  结果中包括的标注类型数量, 如为 0  则不包含任何 annote 信息
    segresult_annote_def* annote_define;  //  对应的标注信息

    u4   token_count;                     //  切分出的 Token 数量
    u4   token_total_count;               //  切分出的全部 Token 数量, 包括额外的切分结果部分
    u8*  token_bitmap;                    //  切分结果
    u4   token_subseq_count;              //  对切分结果的额外补充, 某段, 如果没有启动 nbest, 则为 0
    segresult_token_subseq_list*  token_subseq_list;   //  额外的切分结果

    // u2*  part_of_speech;               //  词性标注的数据, 长度为 token_total_count; 可以为NULL; 词性独立出来,不处理为 Annote 为了压缩
    segresult_pos_list* pos_list;         //  词性标注的列表数据, 生成结果时使用, 反序列化后, 为 NULL

    u4   annote_count;                    //  标注的数量
    u4   annote_data_length;              //  全部已知的 annote 的 annote_data_size 的 SUM
    // segresult_annote* annotes;         //  需要的标注数量
    segresult_annote_list* annote_list;   //  运行时 采集 标注信息的结构
    segresult_annote_pool_list* annote_data_pool;   //运行时, annote 实际存数据的...

}segresult;

segresult*  segresult_new(u8 taskid_, u4 text_icode_length_, u4 text_crc32_);       // 创建新结果
void        segresult_free(segresult* segrs_);                                      // 释放切分结果

// 标注 定义有关
void        segresult_set_annote_define(segresult* segrs_, const char* define_);    // 定义 结果中, 应该包含那些 annote 类型
const char* segresult_get_annote_define(segresult* segrs_);                         // 读取 定义好的 annote

// 分词结果
u4          segresult_gettoken_size(segresult* segrs_);                             // 得到保存分词结果 ( 默认 ) 的字节数
u4          segresult_gettoken_count(segresult* segrs_);                            // 目前全部的 token 的数量
int         segresult_settoken(segresult* segrs_, u4 start_,
                               const u1* token_rs_, u4 token_rs_len);               // 设置某个范围的切分结果
int         segresult_settoken_subseq(segresult* segrs_, u4 start_,
                             const u1* token_rs_, u4 token_rs_len);                 // 设置某段的切分结果, 用于 nbest 的情况
// 词性标注结果
int         segresult_setpos(segresult* segrs_, u4 start_,
                               const u2* pos_, u4 pos_len);                         // 设置某个范围的词性标注信息, 如果 pos_list == NULL 并且 start_ !=0 则 不处理

// 标引相关
segresult_annote*  segresult__annote_new(segresult* segrs_);                        // 从当前的 annote 池中分配一个
int         segresult_annote_set(segresult_annote* note_, u4 token_id_, u2 annote_type_id_, u1* data_, u2 data_size_);  // 设置 annote 的数据, 未分配的, token_id 为 0
u4          segresult_annote_count(segresult* segrs_);                              // 目前全部的 Annote 的数量

// 序列化
u4          segresult_serialize_size(segresult* segrs_);                            // 得到结果对象完全序列化后, 需要的数据长度
int         segresult_serialize(segresult* segrs_, u1* data, u4 data_len);          // 把当前结果序列化到内存
int         segresult_unserialize(segresult* segrs_, const u1* data, u4 data_len);  // 从内存中, 重新构造

#endif // SEG_RESULT_H
