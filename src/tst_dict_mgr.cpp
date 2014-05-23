#include <stdio.h>
#include <gtest/gtest.h>

#if _MSC_VER
#define snprintf _snprintf
#endif

#include "mm_dict_base.h"
#include "mm_dict_mgr.h"


int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    RUN_ALL_TESTS();
	return 0;
}

TEST(TestDictMgr, DictLoadTest)
{
	mm::DictMgr mgr;

    mgr.LoadTerm(".");
    mgr.LoadPharse(".");
    mgr.LoadSpecial(".");

    mgr.BuildIndex();
}

TEST(TestDictMgr, TermQueryTest)
{

}

/* -- end of file --  */
