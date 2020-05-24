#include <cxxtest/TestSuite.h>
#include "engines/ultima/ultima8/filesys/idata_source.h"
/**
 * Test suite for the functions in engines/ultima/ultima8/filesys/idata_source.h
 */

class U8IDataSourceTestSuite : public CxxTest::TestSuite {
	public:
	U8IDataSourceTestSuite() {
	}

	void test_empty_ibuffer_source() {
		Ultima::Ultima8::IBufferDataSource source(NULL, 0, false, false);

		TS_ASSERT_EQUALS(source.size(), 0);
		TS_ASSERT_EQUALS(source.pos(), 0);
		TS_ASSERT(source.eos());
	}

	void test_ibuffer_source() {
		uint8 buf[256];
		for (int i = 0; i < 255; i++) {
			buf[i] = 0x80 + i+1;
		}

		Ultima::Ultima8::IBufferDataSource source(buf, 256, false, false);
		TS_ASSERT_EQUALS(source.size(), 256);
		TS_ASSERT_EQUALS(source.pos(), 0);
		TS_ASSERT(!source.eos());

		TS_ASSERT_EQUALS(source.readByte(), 0x81);
		TS_ASSERT_EQUALS(source.readUint16LE(), 0x8382);
		TS_ASSERT_EQUALS(source.readUint24LE(), 0x868584);
		TS_ASSERT_EQUALS(source.readUint32LE(), 0x8A898887);
		source.skip(-2);
		TS_ASSERT_EQUALS(source.readUint16LE(), 0x8A89);
		source.seek(16);
		TS_ASSERT_EQUALS(source.readUint16LE(), 0x9291);
		TS_ASSERT_EQUALS(source.readX(1), 0x93);
		TS_ASSERT_EQUALS(source.readX(3), 0x969594);
		TS_ASSERT_EQUALS(source.readXS(1), (int8)0x97);
		TS_ASSERT_EQUALS(source.readXS(3), (int32)0xFF9A9998);
		source.seek(256);
		TS_ASSERT(source.eos());
	}

	void test_ibuffer_str_source() {
		/*
		const char *buf = "this is a \r\n dos string and a \n unix string.";
		Ultima::Std::string str;
		Ultima::Ultima8::IBufferDataSource source(buf, strlen(buf), true, false);
		TS_ASSERT(!source.eos());
		source.readline(str);
		TS_ASSERT(!source.eos());
		TS_ASSERT_EQUALS(str, "this is a ");
		source.readline(str);
		TS_ASSERT_EQUALS(str, " dos string and a ");
		source.readline(str);
		TS_ASSERT_EQUALS(str, " unix string.");
		TS_ASSERT(source.eos());
		*/
	}

};
