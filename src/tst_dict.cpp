#include <stdio.h>
#include <gtest/gtest.h>

#if _MSC_VER
#define snprintf _snprintf
#endif

#include "mm_dict_base.h"

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    RUN_ALL_TESTS();
    return 0;
}

TEST(DictBaseTest, NewTermPropTest)
{
    /*
     * 测试创建 新词典， 用于采用 utf-8 编码， 没有特别设计中文的测试程序
     *  增加 100 条记录
     */
    mm::DictBase dict;
    char buffer [256];
    int cx;

    dict.Init("com.coreseek.test", "id:4;freq:4");

    //for(int i = 0; i < 100000; i++)
    mm::EntryData* entry = NULL;
    for(int i = 0; i < 100; i++)        // simple & fast @develop stage.
    {
        cx = snprintf( buffer, 256, "term_%d", i );
        buffer[cx] = 0;
        entry = dict.Insert(buffer, cx);
        EXPECT_NE(entry, (mm::EntryData*)NULL);
        // FIXME: change to MARCO
        entry->SetU4(dict.GetSchema(), 0, i);
        entry->SetU4(dict.GetSchema(), "freq", 1);
        //entry->SetDataIdx(dict.GetSchema(), dict.GetStringPool(), 1, (const u1*)buffer, cx);
    }

    // try get property before save
    {
        cx = snprintf( buffer, 256, "term_%d", 2 );
        buffer[cx] = 0;
        entry = dict.GetEntryData(buffer, cx);
        EXPECT_NE(entry, (mm::EntryData*)NULL); 
        EXPECT_EQ(entry->GetU4(dict.GetSchema(), 0 ), 2); // id
        EXPECT_EQ(entry->GetU4(dict.GetSchema(), 1 ), 1); // freq
    } 

    // test exactly match
    {
        int rs = dict.ExactMatch(buffer, cx);
        int eoff = dict.GetEntryOffset(buffer, cx);
        EXPECT_EQ(rs, eoff);

        entry = dict.GetEntryDataByOffset(eoff);
        EXPECT_EQ(entry->GetU4(dict.GetSchema(), 0 ), 2); // id
        EXPECT_EQ(entry->GetU4(dict.GetSchema(), 1 ), 1); // freq
    }
}

TEST(DictBaseTest, NewTermTest)
{
    /*
     * 测试创建 新词典， 用于采用 utf-8 编码， 没有特别设计中文的测试程序
     *  增加 100 条记录
     */
    mm::DictBase dict;
    char buffer [256];
    int cx;

    dict.Init("com.coreseek.test", "id:4;pinyin:s;thres:s;pos:2");

    //for(int i = 0; i < 100000; i++)
    mm::EntryData* entry = NULL;
    for(int i = 0; i < 100; i++)        // simple & fast @develop stage.
    {
        cx = snprintf( buffer, 256, "term_%d", i );
        buffer[cx] = 0;
        entry = dict.Insert(buffer, cx);
        EXPECT_NE(entry, (mm::EntryData*)NULL);
        // FIXME: change to MARCO
        entry->SetU4(dict.GetSchema(), 0, i);
        entry->SetU2(dict.GetSchema(), "pos", i/100);
        entry->SetDataIdx(dict.GetSchema(), dict.GetStringPool(), 1, (const u1*)buffer, cx);
    }

    // try insert the same key
    {
        cx = snprintf( buffer, 256, "term_%d", 2 );
        buffer[cx] = 0;
        entry = dict.Insert(buffer, cx);
        EXPECT_EQ(entry, (mm::EntryData*)NULL);  // return NULL if existed. 
    }
    // try get property before save
    {
        cx = snprintf( buffer, 256, "term_%d", 2 );
        buffer[cx] = 0;
        entry = dict.GetEntryData(buffer, cx);
        EXPECT_NE(entry, (mm::EntryData*)NULL); 
        EXPECT_EQ(entry->GetU4(dict.GetSchema(), 0 ), 2); // id

        u2 data_len = 0;
        const char* sptr = (const char*)entry->GetData(dict.GetSchema(), dict.GetStringPool(), 1, &data_len);
        EXPECT_EQ(data_len, cx);
        EXPECT_EQ(strncmp( sptr, buffer, cx ), 0);
    } 
    // test exactly match
    {
        int rs = dict.ExactMatch(buffer, cx);
        int eoff = dict.GetEntryOffset(buffer, cx);
        EXPECT_EQ(rs, eoff);

        // test mismatch
        buffer[0] = 'w';
        rs = dict.ExactMatch(buffer, cx);
        EXPECT_EQ(rs, -1);
    }
}

TEST(DictBaseTest, DictSaveLoadTest)
{
    /*
     * 测试创建 新词典， 用于采用 utf-8 编码， 没有特别设计中文的测试程序
     *  增加 100 条记录
     *    - Save to disk
     *    - Load from disk
     */
    mm::DictBase dict;
    char buffer [256];
    int cx;

    dict.Init("com.coreseek.test", "id:4;pinyin:s;thres:s;pos:2");

    mm::EntryData* entry = NULL;

    //for(int i = 0; i < 1000000; i++)
    for(int i = 0; i < 200; i++)        // simple & fast @develop stage.
    {
        cx = snprintf( buffer, 256, "term_%d", i );
        buffer[cx] = 0;
        entry = dict.Insert(buffer, cx);
        EXPECT_NE(entry, (mm::EntryData*)NULL);
        // FIXME: change to MARCO
        entry->SetU4(dict.GetSchema(), 0, i);
        entry->SetU2(dict.GetSchema(), "pos", i/100);
        entry->SetDataIdx(dict.GetSchema(), dict.GetStringPool(), 1, (const u1*)buffer, cx);
    }

    // do save
    {
        dict.Save("_tst_dict_saveload.bin", 2012 );  // rev = 2012
        dict.Load("_tst_dict_saveload.bin");
    }
    // check search
    {
        cx = snprintf( buffer, 256, "term_%d", 2 );
        buffer[cx] = 0;
        int rs = dict.ExactMatch(buffer, cx);
        EXPECT_GE(rs, 0);

        // get property data.
        entry = dict.GetEntryDataByOffset(rs);
        EXPECT_NE(entry, (mm::EntryData*)NULL); 
        EXPECT_EQ(entry->GetU4(dict.GetSchema(), 0 ), 2); // id

        u2 data_len = 0;
        const char* sptr = (const char*)entry->GetData(dict.GetSchema(), dict.GetStringPool(), 1, &data_len);
        EXPECT_EQ(data_len, cx);
        EXPECT_EQ(strncmp( sptr, buffer, cx ), 0);
    }
    // check prefix match
    {
        u1 buf[4096];
        mm::DictMatchResult rs(buf, 4096/sizeof(mm::DictMatchEntry));
        cx = snprintf( buffer, 256, "term_%d", 111);
        buffer[cx] = 0;
        int num = dict.PrefixMatch(buffer, cx, &rs);
        EXPECT_GE(num, 3);
    }
}

/* -- end of file --  */
