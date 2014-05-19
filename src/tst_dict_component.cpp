#include <stdio.h>
#include <gtest/gtest.h>

#if _MSC_VER
#define snprintf _snprintf
#endif

#include "mm_stringpool.h"
#include "mm_dict_schema.h"
#include "mm_entry_datapool.h"
#include "mm_entrydata.h"

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    RUN_ALL_TESTS();
	return 0;
}

TEST(StringPoolTest, AllocStringTest)
{
    mm::StringPoolMemory sp;
    i4 offset =  sp.AllocString("hello", 5);
    EXPECT_EQ(offset, 0);
    offset =  sp.AllocString(" world", 6);
    EXPECT_EQ(offset, 5+2);
}

TEST(StringPoolTest,GetStringTest)
{
	mm::StringPoolMemory sp;
	i4 offset =  sp.AllocString("hello", 5);
	EXPECT_EQ(offset, 0);
	offset =  sp.AllocString(" world", 6);
	EXPECT_EQ(offset, 5+2);

	{
		u2 string_len = 0;
		const char* ptr = sp.GetString(7, &string_len);
		EXPECT_EQ(string_len, 6);
		EXPECT_EQ(strncmp(ptr, " world", string_len), 0);
	}
}

TEST(StringPoolTest, GetStringNewPoolTest)
{
	mm::StringPoolMemory sp;
	u4 total_size = 0;
	{
		// make lots of string.
		char buffer [256];
		int cx;
		for(int i=0; i < 15536; i ++ ) {
			cx = snprintf( buffer, 256, "The number of %d", i ); //avoid the hash working.
			sp.AllocString(buffer, cx);
			total_size += sizeof(u2) + cx;
		}
	}

	i4 offset =  sp.AllocString(" world", 6);
	total_size +=  sizeof(u2) + 6;
	//EXPECT_EQ(offset, 5+2);
	{
		u2 string_len = 0;
		const char* ptr = sp.GetString(offset, &string_len);
		EXPECT_EQ(string_len, 6);
		EXPECT_EQ(strncmp(ptr, " world", string_len), 0);
	}
	// test for dump  & load ?
	{
		u1* data_pool = (u1*)malloc(sp.GetSize());
		EXPECT_EQ( sp.Dump(data_pool, sp.GetSize()), mm::StringPoolMemory::STATUS_OK );
		mm::StringPoolMemory sp_load;
		sp_load.Load(data_pool, sp.GetSize());

		u2 string_len = 0;
		const char* ptr = sp.GetString(offset, &string_len);
		EXPECT_EQ(string_len, 6);
		EXPECT_EQ(strncmp(ptr, " world", string_len), 0);
	}
	EXPECT_EQ(sp.GetSize(), total_size);
}

//////////////////////////////////////////////////////////////////////////

TEST(DictSchemaTest, SchemaDefineTest)
{
    mm::DictSchema schema;
    schema.InitString("id:4;pinyin:s;thres:s;pos:2");
    // check column define
	EXPECT_EQ(schema.GetColumnDefine(), "id:4;pinyin:s;thres:s;pos:2;");
    // check column count
	EXPECT_EQ(schema.GetColumnCount(), 4);
    // check EntryDataSize
	EXPECT_EQ(schema.GetEntryDataSize(), 14);
    // Check Offset
    EXPECT_EQ(schema.GetColumn("thres")->GetOffset(), 10);  // 8 + mask
}

TEST(DictSchemaTest, SchemaDefineTooManyColumnTest)
{
    /*
     * 定义了超过 15 个 Column, 目前是直接崩溃。
     */
    mm::DictSchema schema;
    //schema.InitString("id:4;pinyin:s;thres:s;pos:2");
    // check column define
    // check column count
    // check EntryDataSize
}

TEST(DictSchemaTest, SchemaDefineFieldMaskTest)
{
    /*
     * 根据字符串，计算读取数据需要的 掩码
     */
    mm::DictSchema schema;
    schema.InitString("id:4;pinyin:s;thres:s;pos:2");
    u2 mask = 0;
	int rs = schema.GetFieldMask("id;thres", &mask);
	EXPECT_EQ(rs, 0);
	EXPECT_EQ(mask, 5); // 0b101
}

//////////////////////////////////////////////////////////////////////////
TEST(EntryPoolTest, IsEntryDataPODTest)
{
    /*
     *  测试 Entry 是否为简单对象类型，是否可以强制转换
     */
    EXPECT_EQ((int)sizeof(mm::EntryData), (int)sizeof(u1)); // pass, as EntryData is alias of _data_ptr, test how to pass the parameter
	char* val = (char*)malloc(7);
	memcpy(val, "hello", 5);
	val[5] = 0;
	{
		// pointer of entry  == pointer of entry's _data_ptr ( type is &u1*)
		mm::EntryData* entryptr = (mm::EntryData*)val;  // a bit trick....
		EXPECT_EQ(strncmp( (char*)GET_ENTRY_DATA(entryptr), "hello", 5), 0);
	}
	free(val);
}

TEST(EntryPoolTest, NewEntryTest)
{
	/*
	 * 测试创建新的 Entry;
	 * Entry 用于存储词条的属性信息， 使用 StringPool 存储字符串信息
	 */
	mm::DictSchema schema;
	schema.InitString("id:4;pinyin:s;thres:s;pos:2");

	mm::EntryDataPool pool(schema.GetEntryDataSize());
	mm::StringPoolMemory sp;

	mm::EntryData* entryptr = pool.NewEntry();
	mm::EntryData* entryptr2 = pool.NewEntry();
	EXPECT_EQ(GET_ENTRY_DATA(entryptr2) - GET_ENTRY_DATA(entryptr), schema.GetEntryDataSize());
    // EntryData* 实际是异化的 u1*
    entryptr->SetU4(&schema, 0, 165536);
    entryptr->SetDataIdx(&schema, &sp, 1, (const u1*)"hello", 5);
    entryptr->SetDataIdx(&schema, &sp, 2, (const u1*)" world", 6);
    entryptr->SetU2(&schema, 3, 65535);
    EXPECT_EQ(entryptr->GetU4(&schema, 0), 165536);
    EXPECT_EQ(entryptr->GetU2(&schema, 3), 65535);
    {
        u2 vsize = 0;
        const char* sptr = (const char*)entryptr->GetData(&schema, &sp, 2, &vsize);
        EXPECT_EQ(strncmp( sptr, " world", 6 ), 0);
    }
}

/* -- end of file --  */
