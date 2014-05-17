#include <stdio.h>
#include <gtest/gtest.h>

#include "mm_stringpool.h"

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(StringPoolTest, AllocStringTest)
{
    mm::StringPoolMemory sp;
    i4 offset =  sp.AllocString("hello", 5);
    EXPECT_EQ(offset, 0);
    offset =  sp.AllocString(" world", 6);
    EXPECT_EQ(offset, 5+2);
}

TEST(DictSchemaTest, SchemaDefineTest)
{
    mm::StringPoolMemory sp;
    i4 offset =  sp.AllocString("hello", 5);
    EXPECT_EQ(offset, 0);
    offset =  sp.AllocString(" world", 6);
    EXPECT_EQ(offset, 5+2);
}

/* -- end of file --  */
