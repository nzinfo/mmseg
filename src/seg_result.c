#include "csr_typedefs.h"
#include "csr_utils.h"
#include "seg_result.h"

segresult*
segresult_new(u8 taskid_, u4 text_icode_length_, u4 text_crc32_) {
     // 创建新结果
    return  NULL;
}

void
segresult_free(segresult* segrs_) {
    // 释放切分结果
}

// 标注 定义有关
void
segresult_set_annote_define(segresult* segrs_, const char* define_) {
    // 定义 结果中, 应该包含那些 annote 类型
}

const char*
segresult_get_annote_define(segresult* segrs_) {
    // 读取 定义好的 annote
    return NULL;
}

// 分词结果
u4
segresult_gettoken_size(segresult* segrs_) {
    // 得到保存分词结果 ( 默认 ) 的字节数
    return 0;
}

u4
segresult_gettoken_count(segresult* segrs_) {
    // 目前全部的 token 的数量
    return 0;
}

int
segresult_settoken(segresult* segrs_, u4 start_,
                               const u1* token_rs_, u4 token_rs_len) {
    // 设置某个范围的切分结果
    return 0;
}

int
segresult_settoken_subseq(segresult* segrs_, u4 start_,
                             const u1* token_rs_, u4 token_rs_len) {
    // 设置某段的切分结果, 用于 nbest 的情况
    return 0;
}

// 词性标注结果
int
segresult_setpos(segresult* segrs_, u4 start_,
                               const u2* pos_, u4 pos_len) {
    // 设置某个范围的词性标注信息, 如果 pos_list == NULL 并且 start_ !=0 则 不处理
    return 0;
}

// 标引相关
segresult_annote*
segresult__annote_new(segresult* segrs_) {
    // 从当前的 annote 池中分配一个
    return NULL;
}

int
segresult_annote_set(segresult_annote* note_, u4 token_id_, u2 annote_type_id_, u1* data_, u2 data_size_) {
    // 设置 annote 的数据, 未分配的, token_id 为 0
    return NULL;
}

u4
segresult_annote_count(segresult* segrs_) {
    // 目前全部的 Annote 的数量
    return 0;
}

// 序列化
u4
segresult_serialize_size(segresult* segrs_) {
    // 得到结果对象完全序列化后, 需要的数据长度
    return 0;
}

int
segresult_serialize(segresult* segrs_, u1* data, u4 data_len) {
    // 把当前结果序列化到内存
    return 0;
}

int
segresult_unserialize(segresult* segrs_, const u1* data, u4 data_len) {
    // 从内存中, 重新构造
    return 0;
}

/* -- end of file -- */
