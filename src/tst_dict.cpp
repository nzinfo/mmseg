#include <stdio.h>
#include <gtest/gtest.h>

#if _MSC_VER
#define snprintf _snprintf
#endif

#include "mm_stringpool.h"
#include "mm_dict_schema.h"

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
		for(int i=0; i < 65536; i ++ ) {
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


/* -- end of file --  */
