#include <cxxtest/TestSuite.h>
#include "engines/ultima/shared/core/str.h"

/**
 * Test suite for the functions in engines/ultima/shared/core/str.h
 */

class UltimaStrSuite : public CxxTest::TestSuite {

	public:
	UltimaStrSuite () {
	}

	void test_index_of() {
		Ultima::Shared::String s = "  a  ";
		TS_ASSERT_EQUALS(s.indexOf(' '), 0);
		TS_ASSERT_EQUALS(s.indexOf('a'), 2);
		TS_ASSERT_EQUALS(s.indexOf('z'), -1);

		s = " alksjdf ][";
		TS_ASSERT_EQUALS(s.indexOf("3245j9083f45"), 5);
		TS_ASSERT_EQUALS(s.indexOf("0123456789"), -1);
	}

	void test_split() {
		Ultima::Shared::String s = "abc,def,,aaa,";
		Ultima::Shared::StringArray a = s.split(',');
		// Note: final empty string is trimmed
		TS_ASSERT_EQUALS(a.size(), 4);
		TS_ASSERT_EQUALS(a[1], "def");
		TS_ASSERT_EQUALS(a[2], "");

		Ultima::Shared::String s2 = "e,";
		a = s.split(s2);
		TS_ASSERT_EQUALS(a.size(), 5);
		TS_ASSERT_EQUALS(a[1], "d");
		TS_ASSERT_EQUALS(a[2], "f");
	}
};
