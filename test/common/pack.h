#include <cxxtest/TestSuite.h>

#include "common/stdafx.h"
#include "common/scummsys.h"

#include <common/pack-start.h>	// START STRUCT PACKING

struct TestStruct {
	uint32 x;
	byte y;
	uint16 z;
	uint32 a;
	byte b;
};

#include <common/pack-end.h>	// END STRUCT PACKING

#define OFFS(type,item) (int)(&((type*)0)->type::item)

class PackTestSuite : public CxxTest::TestSuite
{
	public:
	void test_packing( void )
	{
		TS_ASSERT_EQUALS( sizeof(TestStruct), 4+1+2+4+1 );
	}

	void test_offsets( void )
	{
		TS_ASSERT_EQUALS( OFFS(TestStruct, x), 0 );
		TS_ASSERT_EQUALS( OFFS(TestStruct, y), 4 );
		TS_ASSERT_EQUALS( OFFS(TestStruct, z), 5 );
		TS_ASSERT_EQUALS( OFFS(TestStruct, a), 7 );
		TS_ASSERT_EQUALS( OFFS(TestStruct, b), 11 );
	}
};
