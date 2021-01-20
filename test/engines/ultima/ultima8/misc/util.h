#include <cxxtest/TestSuite.h>
#include "engines/ultima/shared/std/string.h"
#include "engines/ultima/ultima8/misc/util.h"

/**
 * Test suite for the functions in engines/ultima/ultima8/misc/util.h
 */
class U8UtilTestSuite : public CxxTest::TestSuite {

	public:
	U8UtilTestSuite() {
	}

	void test_trim_spaces() {
		Ultima::Std::string s0 = "   ";
		Ultima::Ultima8::TrimSpaces(s0);
		TS_ASSERT_EQUALS(s0, Ultima::Std::string(""));

		Ultima::Std::string s1 = " abc   ";
		Ultima::Ultima8::TrimSpaces(s1);
		TS_ASSERT_EQUALS(s1, Ultima::Std::string("abc"));

		Ultima::Std::string s2 = "def";
		Ultima::Ultima8::TrimSpaces(s2);
		TS_ASSERT_EQUALS(s2, "def");
	}

	void test_tabs_to_spaces() {
		Ultima::Std::string s1 = "\tabc \t ";
		Ultima::Ultima8::TabsToSpaces(s1, 1);
		TS_ASSERT_EQUALS(s1, " abc   ");

		Ultima::Std::string s2 = "def";
		Ultima::Ultima8::TabsToSpaces(s2, 1);
		TS_ASSERT_EQUALS(s2, "def");
	}

	void test_split_string() {
		Ultima::Std::string s1 = "abc,def";
		Ultima::Std::vector<Ultima::Std::string> v1;
		Ultima::Ultima8::SplitString(s1, ',', v1);

		TS_ASSERT_EQUALS(v1.size(), 2);
		TS_ASSERT_EQUALS(v1[0], "abc");
		TS_ASSERT_EQUALS(v1[1], "def");

		Ultima::Std::string s2;
		Ultima::Std::vector<Ultima::Std::string> v2;
		Ultima::Ultima8::SplitString(s2, ',', v1);
		TS_ASSERT_EQUALS(v1.size(), 0);

		Ultima::Std::string s3 = " aa  bb  ";
		Ultima::Ultima8::SplitString(s3, ' ', v1);
		TS_ASSERT_EQUALS(v1.size(), 6);
		TS_ASSERT_EQUALS(v1[0], "");
		TS_ASSERT_EQUALS(v1[1], "aa");
	}

	void test_argv_to_string() {
		// FIXME: It would be nicer if this function didn't leave a trailing space
		Ultima::Std::vector<Ultima::Std::string> v;
		Ultima::Std::string s;
		Ultima::Ultima8::ArgvToString(v, s);
		TS_ASSERT_EQUALS(s, "");

		v.push_back("abc");
		v.push_back("\t\nescape me \"!");
		Ultima::Ultima8::ArgvToString(v, s);
		TS_ASSERT_EQUALS(s, "abc \\t\\nescape\\ me\\ \\\"! ");
	}

	void test_string_to_argv() {
		Ultima::Std::string s = "";
		Ultima::Std::vector<Ultima::Std::string> v;
		Ultima::Ultima8::StringToArgv(s, v);
		TS_ASSERT_EQUALS(v.size(), 0);

		// Test it strips leading space on args, and includes spaces inside ""s
		s = "abc \\t\\nescape \"\\ me\\ \\\" !\" ";
		Ultima::Ultima8::StringToArgv(s, v);
		TS_ASSERT_EQUALS(v.size(), 3);
		TS_ASSERT_EQUALS(v[0], "abc");
		TS_ASSERT_EQUALS(v[1], "escape");
		TS_ASSERT_EQUALS(v[2], " me \" !");
	}
};
