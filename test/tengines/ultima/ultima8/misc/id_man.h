#include <cxxtest/TestSuite.h>
#include "engines/ultima/ultima8/misc/id_man.h"

/**
 * Test suite for the functions in engines/ultima/ultima8/misc/id_man.h
 */

class U8IdManTestSuite : public CxxTest::TestSuite {

	public:
	U8IdManTestSuite() {
	}

	void test_trim_spaces() {
		Ultima::Ultima8::idMan idman = Ultima::Ultima8::idMan(1234, 5678);
		TS_ASSERT(!idman.isFull());
		TS_ASSERT(!idman.isIDUsed(1234));

		uint16 newid = idman.getNewID();
		TS_ASSERT_EQUALS(newid, 1234);
		TS_ASSERT(idman.isIDUsed(1234));

		bool reserved = idman.reserveID(1234);
		TS_ASSERT(!reserved);
		reserved = idman.reserveID(1235);
		TS_ASSERT(reserved);
		TS_ASSERT(idman.isIDUsed(1235));
		uint16 newid2 = idman.getNewID();
		TS_ASSERT_EQUALS(newid2, 1236);
		TS_ASSERT(idman.isIDUsed(1236));

		bool reserved2 = idman.reserveID(2000);
		TS_ASSERT(reserved2);
		TS_ASSERT(idman.isIDUsed(2000));

		idman.clearAll();
		idman.setNewMax(2001);
		TS_ASSERT(!idman.isFull());
		TS_ASSERT(!idman.isIDUsed(2000));
		TS_ASSERT(!idman.isIDUsed(1234));
	}
};
