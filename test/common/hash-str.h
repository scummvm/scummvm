#include <cxxtest/TestSuite.h>
#include "common/hash-str.h"

class HashStrTestSuite : public CxxTest::TestSuite
{

	public:
	void test_case_sensitive_string_equal_to() {
		// Name says it all.
		const Common::String lower("test");
		const Common::String lower1("test");
		const Common::String mixed("tESt");
		const Common::String spaced("test ");
		const Common::String doublespaced("test  ");
		const Common::String tabbed("test\t");

		Common::CaseSensitiveString_EqualTo css_et;
		TS_ASSERT_EQUALS(css_et(lower, mixed), false);
		TS_ASSERT_EQUALS(css_et(lower, lower1), true);
		TS_ASSERT_EQUALS(css_et(lower, lower), true);

		// Different sorts of whitespace are to be treated differently.
		TS_ASSERT_EQUALS(css_et(lower, spaced), false);
		TS_ASSERT_EQUALS(css_et(lower, tabbed), false);
		TS_ASSERT_EQUALS(css_et(spaced, tabbed), false);
		TS_ASSERT_EQUALS(css_et(spaced, doublespaced), false);
	}

	void test_ignore_case_equal_to() {

		// This should be probably called
		// case_insensitive_string_equal_to or something,
		// but it's basically the same thing as the above,
		// only case insensitive.

		const Common::String lower("test");
		const Common::String lower1("test");
		const Common::String mixed("tESt");
		const Common::String spaced("test ");
		const Common::String mixedspaced("tESt ");
		const Common::String doublespaced("test  ");
		const Common::String tabbed("test\t");

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
	}

	void test_case_sensitive_string_hash() {

		// In which we compute string hashes for different strings
		// and see that the functor is case sensitive and does ok with spaces.

		const Common::String lower("test");
		uint lower_hash = 1308370872; // CPython told me so
		const Common::String lower1("test");
		const Common::String mixed("tESt");
		uint mixed_hash = -1217273608;
		const Common::String spaced("test ");
		uint spaced_hash = -1086267887;
		const Common::String mixedspaced("tESt ");
		const Common::String doublespaced("test  ");
		const Common::String tabbed("test\t");
		uint tabbed_hash = -1086267848;

		Common::CaseSensitiveString_Hash css_h;
		TS_ASSERT_EQUALS(css_h(lower), lower_hash);
		TS_ASSERT_EQUALS(css_h(mixed), mixed_hash);
		TS_ASSERT_EQUALS(css_h(spaced), spaced_hash);
		TS_ASSERT_EQUALS(css_h(tabbed), tabbed_hash);
		TS_ASSERT_DIFFERS(css_h(spaced), css_h(doublespaced));
	}

	void test_ignore_case_hash() {
		// Same, but case insensitive.
		const Common::String lower("test");
		uint lower_hash = 1308370872; // CPython told me so
		const Common::String lower1("test");
		const Common::String mixed("tESt");
		const Common::String spaced("test ");
		uint spaced_hash = -1086267887;
		const Common::String mixedspaced("tESt ");
		const Common::String doublespaced("test  ");
		const Common::String tabbed("test\t");
		uint tabbed_hash = -1086267848;

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

		const Common::String lower("test");
		uint lower_hash = 1308370872; // CPython told me so
		const Common::String lower1("test");
		const Common::String mixed("tESt");
		uint mixed_hash = -1217273608;
		const Common::String spaced("test ");
		uint spaced_hash = -1086267887;
		const Common::String mixedspaced("tESt ");
		const Common::String doublespaced("test  ");
		const Common::String tabbed("test\t");
		uint tabbed_hash = -1086267848;

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
		// We run the same tests with Hash<const char*>,
		// a template specialization of Hash, also a functor,
		// that works with C-Style strings.
		// It is supposed to be case sensitive.

		char lower[] = "test";
		uint lower_hash = 1308370872; // CPython told me so
		char lower1[] = "test";
		char mixed[] = "tESt";
		uint mixed_hash = -1217273608;
		char spaced[] = "test ";
		uint spaced_hash = -1086267887;
		char mixedspaced[] = "tESt ";
		char doublespaced[] = "test  ";
		char tabbed[] = "test\t";
		uint tabbed_hash = -1086267848;

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
