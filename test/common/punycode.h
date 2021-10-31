#include <cxxtest/TestSuite.h>
#include "common/punycode.h"

/**
 * Test suite for the functions in common/util.h
 */
static const char *strings[] = {
	"Icon\r", "xn--Icon-ja6e", "1",
	"ascii", "ascii", "0",
	"ends with dot .", "xn--ends with dot .-", "1",
	"ends with space ", "xn--ends with space -", "1",
	"バッドデイ(Power PC)", "xn--(Power PC)-jx4ilmwb1a7h", "1",
	"Hello*", "xn--Hello-la10a", "1",
	0
};

class PunycodeTestSuite : public CxxTest::TestSuite {
	public:

	void test_punycode() {
		for (const char **a = strings; *a; a += 3) {
			Common::U32String string_in(a[0]);
			Common::String string_out(a[1]);
			bool need = (a[2][0] == '1');

			TS_ASSERT_EQUALS(punycode_decodefilename(string_out), string_in);
			TS_ASSERT_EQUALS(punycode_encodefilename(string_in), string_out);
			TS_ASSERT_EQUALS(punycode_needEncode(string_in), need);

			warning("'%s' -> '%s'", string_in.encode().c_str(), punycode_encodefilename(string_in).c_str());
		}
	}
};
