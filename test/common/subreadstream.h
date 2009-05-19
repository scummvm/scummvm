#include <cxxtest/TestSuite.h>

#include "common/stream.h"

class SubReadStreamTestSuite : public CxxTest::TestSuite {
	public:
	void test_traverse() {
		byte contents[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		Common::MemoryReadStream ms(contents, 10);

		int end = 5;

		Common::SubReadStream srs(&ms, end);

		int i;
		byte b;
		for (i = 0; i < end; ++i) {
			TS_ASSERT(!srs.eos());

			b = srs.readByte();
			TS_ASSERT_EQUALS(i, b);
		}

		TS_ASSERT(!srs.eos());
		b = srs.readByte();
		TS_ASSERT(srs.eos());
	}
};
