#include <cxxtest/TestSuite.h>
#include "engines/ultima/ultima8/usecode/bit_set.h"
/**
 * Test suite for the functions in engines/ultima/ultima8/usecode/bit_set.h
 */

class U8BitSetTestSuite : public CxxTest::TestSuite {
	public:
	Ultima::Ultima8::BitSet bs;

	U8BitSetTestSuite() {
		bs.setSize(0x1000);
	}

	void test_set_get() {
		// Test with bit pattern in second byte of 01110100
		// (pos goes from low bit to high bit)
		bs.setEntries(10, 5, 0x1D);
		TS_ASSERT_EQUALS(bs.getEntries(10, 5), 0x1Du);
		TS_ASSERT_EQUALS(bs.getEntries(10, 4), 0xDu);
		TS_ASSERT_EQUALS(bs.getEntries(8, 6), 0xDu << 2);
		TS_ASSERT_EQUALS(bs.getEntries(8, 7), 0x1Du << 2);
		TS_ASSERT_EQUALS(bs.getEntries(8, 8), 0x1Du << 2);
		TS_ASSERT_EQUALS(bs.getEntries(14, 2), 0x1u);
		TS_ASSERT_EQUALS(bs.getEntries(16, 32), 0u);
		TS_ASSERT_EQUALS(bs.getEntries(0, 10), 0u);
	}

	void test_clear() {
		bs.setEntries(10, 5, 0x15);
		bs.setSize(0x1000);
		TS_ASSERT_EQUALS(bs.getEntries(10, 5), 0u);
		TS_ASSERT_EQUALS(bs.getEntries(0, 32), 0u);
	}
};
