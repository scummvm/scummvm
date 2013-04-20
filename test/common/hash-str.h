#include <cxxtest/TestSuite.h>
#include "common/hash-str.h"

class HashStrTestSuite : public CxxTest::TestSuite
{
	/*
	 * Test suite for hashstr.
	 */

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
		const Common::String plus128("t\xE5est");
		// 'e'+128 = 0xE5

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
		const Common::String plus128("t\xE5est");

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
		// The hashes come from Python's hash() function.

		const Common::String lower("test");
		const uint lower_hash = 1308370872;
		const Common::String lower1("test");
		const Common::String mixed("tESt");
		const uint mixed_hash = -1217273608;
		const Common::String spaced("test ");
		const uint spaced_hash = -1086267887;
		const Common::String mixedspaced("tESt ");
		const Common::String doublespaced("test  ");
		const Common::String tabbed("test\t");
		const uint tabbed_hash = -1086267848;

		Common::CaseSensitiveString_Hash css_h;
		TS_ASSERT_EQUALS(css_h(lower), lower_hash);
		TS_ASSERT_EQUALS(css_h(mixed), mixed_hash);
		TS_ASSERT_EQUALS(css_h(spaced), spaced_hash);
		TS_ASSERT_EQUALS(css_h(tabbed), tabbed_hash);
		TS_ASSERT_DIFFERS(css_h(spaced), css_h(doublespaced));
	}

	void test_ignore_case_hash() {
		// Same as test_case_sensitive_string_hash, but case insensitive.
		const Common::String lower("test");
		const uint lower_hash = 1308370872;
		const Common::String lower1("test");
		const Common::String mixed("tESt");
		const Common::String spaced("test ");
		const uint spaced_hash = -1086267887;
		const Common::String mixedspaced("tESt ");
		const Common::String doublespaced("test  ");
		const Common::String tabbed("test\t");
		const uint tabbed_hash = -1086267848;

		Common::IgnoreCase_Hash ic_h;
		TS_ASSERT_EQUALS(ic_h(lower), lower_hash);
		TS_ASSERT_EQUALS(ic_h(mixed), lower_hash);
		TS_ASSERT_EQUALS(ic_h(spaced), spaced_hash);
		TS_ASSERT_EQUALS(ic_h(tabbed), tabbed_hash);
		TS_ASSERT_EQUALS(ic_h(mixedspaced), spaced_hash);
		TS_ASSERT_DIFFERS(ic_h(spaced), ic_h(doublespaced));
	}

	void test_cpp_string_hash ()
	{
		// We run the same tests with Hash<String>,
		// a template specialization of Hash, also a functor.
		// It is supposed to be case sensitive.
		// Again, hashes come from Python's hash().

		const Common::String lower("test");
		const uint lower_hash = 1308370872;
		const Common::String lower1("test");
		const Common::String mixed("tESt");
		const uint mixed_hash = -1217273608;
		const Common::String spaced("test ");
		const uint spaced_hash = -1086267887;
		const Common::String mixedspaced("tESt ");
		const Common::String doublespaced("test  ");
		const Common::String tabbed("test\t");
		const uint tabbed_hash = -1086267848;

		Common::Hash<Common::String> h;
		TS_ASSERT_EQUALS(h(lower), lower_hash);
		TS_ASSERT_EQUALS(h(lower), h(lower1));
		TS_ASSERT_EQUALS(h(mixed), mixed_hash);
		TS_ASSERT_EQUALS(h(spaced), spaced_hash);
		TS_ASSERT_EQUALS(h(tabbed), tabbed_hash);
		TS_ASSERT_DIFFERS(h(spaced), h(doublespaced));
	}

	void test_c_style_string_hash ()
	{
		// Same as test_cpp_string_hash but with Hash<const char*>,
		// a template specialization of Hash, also a functor,
		// that works with C-Style strings.
		// It is supposed to be case sensitive.

		char lower[] = "test";
		const uint lower_hash = 1308370872; // CPython told me so
		char lower1[] = "test";
		char mixed[] = "tESt";
		const uint mixed_hash = -1217273608;
		char spaced[] = "test ";
		const uint spaced_hash = -1086267887;
		char mixedspaced[] = "tESt ";
		char doublespaced[] = "test  ";
		char tabbed[] = "test\t";
		const uint tabbed_hash = -1086267848;

		Common::Hash<const char*> h;
		TS_ASSERT_EQUALS(h(lower), lower_hash);
		TS_ASSERT_EQUALS(h(lower), h(lower1));
		TS_ASSERT_EQUALS(h(mixed), mixed_hash);
		TS_ASSERT_EQUALS(h(spaced), spaced_hash);
		TS_ASSERT_DIFFERS(h(spaced), h(mixedspaced));
		TS_ASSERT_EQUALS(h(tabbed), tabbed_hash);
		TS_ASSERT_DIFFERS(h(spaced), h(doublespaced));
	}


};
