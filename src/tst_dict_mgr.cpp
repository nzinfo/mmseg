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

    // test query
    {
        u1 buf[4096];
        mm::DictMatchResult rs(buf, 4096/sizeof(mm::DictMatchEntry));
        // Chinese characters for "zhongwen" ("Chinese language").
        const char kChineseSampleText[] = {-28, -72, -83, -26, -106, -121, 0};
        int n = mgr.ExactMatch(kChineseSampleText, 6, &rs);
		printf("find %d hits\n", n);
		for(int i=0; i<n; i++) {
			printf("dict_id %d, rs=%d ", rs.GetMatch(i)->match._dict_id, rs.GetMatch(i)->match._value);
			// dump pinyin ,  std mmseg have no pinyin , should output as NULL.
			mm::DictBase* dict = mgr.GetDictionary( rs.GetMatch(i)->match._dict_id );

			mm::EntryData* entry = dict->GetEntryDataByOffset( rs.GetMatch(i)->match._value );
			std::string s = dict->GetSchema()->GetColumnDefine();
			printf("schema = %s name= %s \n", s.c_str(), dict->GetDictName().c_str() );
			const mm::DictSchemaColumn* column = dict->GetSchema()->GetColumn("pinyin");
			if(column) {
				u2 data_len = 0;
				const char* sptr = (const char*)entry->GetData(dict->GetSchema(), dict->GetStringPool(), column->GetIndex(), &data_len);
				printf("%*.*s/x ", data_len, data_len, sptr);
			}
		}
    }
}

TEST(TestDictMgr, TermQueryTest)
{

}

/* -- end of file --  */
