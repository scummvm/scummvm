#include <cxxtest/TestSuite.h>
#include "engines/ultima/ultima8/filesys/odata_source.h"
/**
 * Test suite for the functions in engines/ultima/ultima8/filesys/odata_source.h
 */

class U8ODataSourceTestSuite : public CxxTest::TestSuite {
	public:
	U8ODataSourceTestSuite() {
	}

	void test_autobuffer_source() {
		Ultima::Ultima8::OAutoBufferDataSource source(12);
		TS_ASSERT_EQUALS(source.size(), 0);
		TS_ASSERT_EQUALS(source.pos(), 0);

		source.writeByte(0xBEEF);
		TS_ASSERT_EQUALS(source.size(), 1);
		TS_ASSERT_EQUALS(source.pos(), 1);

		for (int i = 0; i < 10; i++) {
			source.writeUint32LE(0x8088C0DE);
		}
		TS_ASSERT_EQUALS(source.size(), 41);
		TS_ASSERT_EQUALS(source.pos(), 41);
		source.skip(0);
		TS_ASSERT_EQUALS(source.size(), 41);
		TS_ASSERT_EQUALS(source.pos(), 41);
		// Check trying to skip past the end
		source.skip(2);
		TS_ASSERT_EQUALS(source.size(), 41);
		TS_ASSERT_EQUALS(source.pos(), 41);
		source.skip(-2);
		TS_ASSERT_EQUALS(source.pos(), 39);
		source.writeByte(0x99);
		TS_ASSERT_EQUALS(source.size(), 41);
		TS_ASSERT_EQUALS(source.pos(), 40);
		source.seek(2);
		TS_ASSERT_EQUALS(source.size(), 41);
		TS_ASSERT_EQUALS(source.pos(), 2);

		const uint8* buf = source.getData();
		TS_ASSERT_EQUALS(buf[0], 0xEF);
		TS_ASSERT_EQUALS(buf[1], 0xDE);
		TS_ASSERT_EQUALS(buf[2], 0xC0);
		TS_ASSERT_EQUALS(buf[3], 0x88);
		TS_ASSERT_EQUALS(buf[23], 0x88);
		TS_ASSERT_EQUALS(buf[24], 0x80);
		TS_ASSERT_EQUALS(buf[35], 0x88);
		TS_ASSERT_EQUALS(buf[36], 0x80);

		source.clear();
		TS_ASSERT_EQUALS(source.size(), 0);
		TS_ASSERT_EQUALS(source.pos(), 0);

		source.writeByte(0x04030201);
		source.writeUint16LE(0x08070605);
		source.write3(0x0C0B0A09);
		source.writeUint32LE(0x100F0E0D);

		buf = source.getData();
		TS_ASSERT_EQUALS(buf[0], 0x01);
		TS_ASSERT_EQUALS(buf[1], 0x05);
		TS_ASSERT_EQUALS(buf[2], 0x06);
		TS_ASSERT_EQUALS(buf[3], 0x09);
		TS_ASSERT_EQUALS(buf[4], 0x0A);
		TS_ASSERT_EQUALS(buf[5], 0x0B);
		TS_ASSERT_EQUALS(buf[6], 0x0D);
		TS_ASSERT_EQUALS(buf[7], 0x0E);
		TS_ASSERT_EQUALS(buf[8], 0x0F);
		TS_ASSERT_EQUALS(buf[9], 0x10);
	}

};
