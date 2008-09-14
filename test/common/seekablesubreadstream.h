#include <cxxtest/TestSuite.h>

#include "common/stream.h"

class SeekableSubReadStreamTestSuite : public CxxTest::TestSuite {
	public:
	void test_traverse(void) {
		byte contents[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		Common::MemoryReadStream ms(contents, 10);

		int start = 2, end = 8;

		Common::SeekableSubReadStream ssrs(&ms, start, end);

		int i;
		byte b;
		for (i = start; i < end; ++i) {
			TS_ASSERT( !ssrs.eos() );

			TS_ASSERT_EQUALS( uint32(i - start), ssrs.pos() );

			ssrs.read(&b, 1);
			TS_ASSERT_EQUALS( i, b );
		}

		TS_ASSERT( !ssrs.eos() );
		TS_ASSERT( 0 == ssrs.read(&b, 1) );
		TS_ASSERT( ssrs.eos() );
	}

	void test_seek(void) {
		byte contents[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		Common::MemoryReadStream ms(contents, 10);

		Common::SeekableSubReadStream ssrs(&ms, 1, 9);
		byte b;

		TS_ASSERT_EQUALS( ssrs.pos(), (uint32)0 );

		ssrs.seek(1, SEEK_SET);
		TS_ASSERT_EQUALS( ssrs.pos(), (uint32)1 );
		b = ssrs.readByte();
		TS_ASSERT_EQUALS( b, 2 );

		ssrs.seek(5, SEEK_CUR);
		TS_ASSERT_EQUALS( ssrs.pos(), (uint32)7 );
		b = ssrs.readByte();
		TS_ASSERT_EQUALS( b, 8 );

		ssrs.seek(-3, SEEK_CUR);
		TS_ASSERT_EQUALS( ssrs.pos(), (uint32)5 );
		b = ssrs.readByte();
		TS_ASSERT_EQUALS( b, 6 );

		ssrs.seek(0, SEEK_END);
		TS_ASSERT_EQUALS( ssrs.pos(), (uint32)8 );
		TS_ASSERT( !ssrs.eos() );
		b = ssrs.readByte();
		TS_ASSERT( ssrs.eos() );

		ssrs.seek(3, SEEK_END);
		TS_ASSERT( !ssrs.eos() );
		TS_ASSERT_EQUALS( ssrs.pos(), (uint32)5 );
		b = ssrs.readByte();
		TS_ASSERT_EQUALS( b, 6 );

		ssrs.seek(8, SEEK_END);
		TS_ASSERT_EQUALS( ssrs.pos(), (uint32)0 );
		b = ssrs.readByte();
		TS_ASSERT_EQUALS( b, 1 );
	}
};
