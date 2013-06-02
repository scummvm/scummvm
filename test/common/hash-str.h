#include <cxxtest/TestSuite.h>
#include "common/hash-str.h"

/**
 * Test suite for common/hash-str.h
 * We test a number of case sensitive/insensitive hash and compare functions
 * using example strings and known hashes, trying to tackle
 * as much edge cases as possible.
 */
class HashStrTestSuite : public CxxTest::TestSuite {

	public:
	void test_case_sensitive_string_equal_to() {

		// Name says it all.
		// This verifies that the function returns true
		// for exactly the same string, false for the same
		// string in mixed case and false for some edge cases
		// with various spacings plus one character replaced
		// by itself+128 (if there's some processing done after
		// conversion to 7-bit ASCII this might yield funny results).

		const Common::String lower("test");
		const Common::String lower1("test");
		const Common::String mixed("tESt");
		const Common::String spaced("test ");
		const Common::String doublespaced("test  ");
		const Common::String tabbed("test\t");
		const Common::String plus128("t\345est");
		// 'e'+128 = 0xE5 = 0o345

		Common::CaseSensitiveString_EqualTo css_et;
		TS_ASSERT_EQUALS(css_et(lower, mixed), false);
		TS_ASSERT_EQUALS(css_et(lower, lower1), true);
		TS_ASSERT_EQUALS(css_et(lower, lower), true);

		// Different sorts of whitespace are to be treated differently.
		TS_ASSERT_EQUALS(css_et(lower, spaced), false);
		TS_ASSERT_EQUALS(css_et(lower, tabbed), false);
		TS_ASSERT_EQUALS(css_et(spaced, tabbed), false);
		TS_ASSERT_EQUALS(css_et(spaced, doublespaced), false);
		TS_ASSERT_EQUALS(css_et(lower, plus128), false);
	}

	void test_ignore_case_equal_to() {

		// This should be probably called case_insensitive_string_equal_to
		// or something,but it's basically the same thing as
		// test_case_sensitive_string_equal_to, only it's case
		// insensitive.

		const Common::String lower("test");
		const Common::String lower1("test");
		const Common::String mixed("tESt");
		const Common::String spaced("test ");
		const Common::String mixedspaced("tESt ");
		const Common::String doublespaced("test  ");
		const Common::String tabbed("test\t");
		const Common::String plus128("t\345est");

		Common::IgnoreCase_EqualTo ic_et;
		TS_ASSERT_EQUALS(ic_et(lower, mixed), true);
		TS_ASSERT_EQUALS(ic_et(lower, lower1), true);
		TS_ASSERT_EQUALS(ic_et(lower, lower), true);
		// Edge case:
		TS_ASSERT_EQUALS(ic_et(spaced, mixedspaced), true);

		// Different sorts of whitespace are to be treated differently.
		TS_ASSERT_EQUALS(ic_et(lower, spaced), false);
		TS_ASSERT_EQUALS(ic_et(lower, tabbed), false);
		TS_ASSERT_EQUALS(ic_et(spaced, tabbed), false);
		TS_ASSERT_EQUALS(ic_et(spaced, doublespaced), false);
		TS_ASSERT_EQUALS(ic_et(lower, plus128), false);
	}

	void test_case_sensitive_string_hash() {

		// Here we compute string hashes for different
		// strings and see that the functor is case sensitive
		// and does not ignore spaces.

		const Common::String lower("test");
		const Common::String lower1("test");
		const Common::String mixed("tESt");
		const Common::String spaced("test ");
		const Common::String mixedspaced("tESt ");
		const Common::String doublespaced("test  ");
		const Common::String tabbed("test\t");

		Common::CaseSensitiveString_Hash css_h;
		TS_ASSERT_EQUALS(css_h(lower), css_h(lower1));
		TS_ASSERT_DIFFERS(css_h(mixed), css_h(lower));
		TS_ASSERT_DIFFERS(css_h(spaced), css_h(lower));
		TS_ASSERT_DIFFERS(css_h(tabbed), css_h(spaced));
		TS_ASSERT_DIFFERS(css_h(spaced), css_h(doublespaced));
	}

	void test_ignore_case_hash() {
		// Same as test_case_sensitive_string_hash, but case insensitive.
		const Common::String lower("test");
		const Common::String lower1("test");
		const Common::String mixed("tESt");
		const Common::String spaced("test ");
		const Common::String mixedspaced("tESt ");
		const Common::String doublespaced("test  ");
		const Common::String tabbed("test\t");

		Common::IgnoreCase_Hash ic_h;
		TS_ASSERT_EQUALS(ic_h(lower), ic_h(lower1));
		TS_ASSERT_EQUALS(ic_h(mixed), ic_h(lower));
		TS_ASSERT_EQUALS(ic_h(spaced), ic_h(mixedspaced));
		TS_ASSERT_DIFFERS(ic_h(tabbed), ic_h(lower));
		TS_ASSERT_DIFFERS(ic_h(spaced), ic_h(doublespaced));
	}

	void test_cpp_string_hash()
	{
		// We run the same tests with Hash<String>,
		// a template specialization of Hash, also a functor.
		// It is supposed to be case sensitive.

		const Common::String lower("test");
		const Common::String lower1("test");
		const Common::String mixed("tESt");
		const Common::String spaced("test ");
		const Common::String mixedspaced("tESt ");
		const Common::String doublespaced("test  ");
		const Common::String tabbed("test\t");

		Common::Hash<Common::String> h;
		TS_ASSERT_EQUALS(h(lower), h(lower1));
		TS_ASSERT_DIFFERS(h(mixed), h(lower));
		TS_ASSERT_DIFFERS(h(spaced), h(lower));
		TS_ASSERT_DIFFERS(h(tabbed), h(spaced));
		TS_ASSERT_DIFFERS(h(spaced), h(doublespaced));
	}

	void test_c_style_string_hash()
	{
		// Same as test_cpp_string_hash but with Hash<const char*>,
		// a template specialization of Hash, also a functor,
		// that works with C-Style strings.
		// It is supposed to be case sensitive.

		char lower[] = "test";
		char lower1[] = "test";
		char mixed[] = "tESt";
		char spaced[] = "test ";
		char mixedspaced[] = "tESt ";
		char doublespaced[] = "test  ";
		char tabbed[] = "test\t";

		Common::Hash<const char *> h;
		TS_ASSERT_EQUALS(h(lower), h(lower1));
		TS_ASSERT_DIFFERS(h(mixed), h(lower));
		TS_ASSERT_DIFFERS(h(spaced), h(lower));
		TS_ASSERT_DIFFERS(h(spaced), h(mixedspaced));
		TS_ASSERT_DIFFERS(h(tabbed), h(spaced));
		TS_ASSERT_DIFFERS(h(spaced), h(doublespaced));

	}


};
