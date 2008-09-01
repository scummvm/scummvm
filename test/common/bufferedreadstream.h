#include <cxxtest/TestSuite.h>

#include "common/stream.h"

class BufferedReadStreamTestSuite : public CxxTest::TestSuite {
	public:
	void test_traverse(void) {
		byte contents[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		Common::MemoryReadStream ms(contents, 10);

		// Use a buffer size of 4 -- note that 10 % 4 != 0,
		// so we test what happens if the cache can't be completly
		// refilled.
		Common::BufferedReadStream srs(&ms, 4);

		byte i, b;
		for (i = 0; i < 10; ++i) {
			TS_ASSERT( !srs.eos() );

			b = srs.readByte();
			TS_ASSERT_EQUALS( i, b );
		}

		TS_ASSERT( srs.eos() );
	}
};
