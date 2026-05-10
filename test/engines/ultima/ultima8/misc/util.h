#include <cxxtest/TestSuite.h>
#include "engines/ultima/ultima8/misc/util.h"

/**
 * Test suite for the functions in engines/ultima/ultima8/misc/util.h
 */
class U8UtilTestSuite : public CxxTest::TestSuite {

	public:
	U8UtilTestSuite() {
	}

	void test_split_string() {
		Common::String s1 = "abc,def";
		Common::Array<Common::String> v1;
		Ultima::Ultima8::SplitString(s1, ',', v1);

		TS_ASSERT_EQUALS(v1.size(), 2u);
		TS_ASSERT_EQUALS(v1[0], "abc");
		TS_ASSERT_EQUALS(v1[1], "def");

		Common::String s2;
		Common::Array<Common::String> v2;
		Ultima::Ultima8::SplitString(s2, ',', v1);
		TS_ASSERT_EQUALS(v1.size(), 0u);

		Common::String s3 = " aa  bb  ";
		Ultima::Ultima8::SplitString(s3, ' ', v1);
		TS_ASSERT_EQUALS(v1.size(), 6u);
		TS_ASSERT_EQUALS(v1[0], "");
		TS_ASSERT_EQUALS(v1[1], "aa");
	}

	void test_string_to_argv() {
		Common::Array<Common::String> v;
		Common::String s;
		Ultima::Ultima8::StringToArgv(s, v);
		TS_ASSERT_EQUALS(v.size(), 0u);

		// Test it strips leading space on args, and includes spaces inside ""s
		s = "abc \\t\\nescape \"\\ me\\ \\\" !\" ";
		Ultima::Ultima8::StringToArgv(s, v);
		TS_ASSERT_EQUALS(v.size(), 3u);
		TS_ASSERT_EQUALS(v[0], "abc");
		TS_ASSERT_EQUALS(v[1], "escape");
		TS_ASSERT_EQUALS(v[2], " me \" !");
	}
};
