#include <cxxtest/TestSuite.h>
#include "engines/ultima/ultima8/usecode/uc_list.h"
/**
 * Test suite for the functions in engines/ultima/ultima8/usecode/uc_list.h
 */

class U8UCListTestSuite : public CxxTest::TestSuite {
	public:
	U8UCListTestSuite() {
	}

	void test_static_list() {
		Ultima::Ultima8::UCList l(2);

		TS_ASSERT_EQUALS(l.getSize(), 0);
		TS_ASSERT_EQUALS(l.getElementSize(), 2);

		uint16 test = 0xBEEF;
		l.append((uint8*)&test);
		TS_ASSERT_EQUALS(l.getSize(), 1);

		uint16 test2 = 0xF00D;
		l.append((uint8*)&test2);
		TS_ASSERT_EQUALS(l.getSize(), 2);
		TS_ASSERT(l.inList((uint8*)&test));

		l.remove((uint8*)&test);
		TS_ASSERT(!l.inList((uint8*)&test));
		TS_ASSERT(l.inList((uint8*)&test2));
		TS_ASSERT_EQUALS(l.getSize(), 1);

		l.free();
		TS_ASSERT_EQUALS(l.getSize(), 0);
	}

};
