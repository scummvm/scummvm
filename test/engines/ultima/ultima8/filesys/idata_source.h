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

		TS_ASSERT_EQUALS(source.getSize(), 0);
		TS_ASSERT_EQUALS(source.getPos(), 0);
		TS_ASSERT(source.eof());
	}

	void test_ibuffer_source() {
		uint8 buf[256];
		for (int i = 0; i < 255; i++) {
			buf[i] = i+1;
		}

		Ultima::Ultima8::IBufferDataSource source(buf, 256, false, false);
		TS_ASSERT_EQUALS(source.getSize(), 256);
		TS_ASSERT_EQUALS(source.getPos(), 0);
		TS_ASSERT(!source.eof());

		TS_ASSERT_EQUALS(source.read1(), 1);
		TS_ASSERT_EQUALS(source.read2(), 0x0302);
		TS_ASSERT_EQUALS(source.read3(), 0x060504);
		TS_ASSERT_EQUALS(source.read4(), 0x0A090807);
		source.skip(-2);
		TS_ASSERT_EQUALS(source.read2(), 0x0A09);
		source.seek(16);
		TS_ASSERT_EQUALS(source.read2(), 0x1211);
		TS_ASSERT_EQUALS(source.readX(1), 0x13);
		TS_ASSERT_EQUALS(source.readX(3), 0x161514);
		TS_ASSERT_EQUALS(source.readXS(1), 0x17);
		TS_ASSERT_EQUALS(source.readXS(3), 0x1A1918);
		source.seek(256);
		TS_ASSERT(source.eof());
	}

	void test_ibuffer_str_source() {
		/*
		const char *buf = "this is a \r\n dos string and a \n unix string.";
		Ultima::Std::string str;
		Ultima::Ultima8::IBufferDataSource source(buf, strlen(buf), true, false);
		TS_ASSERT(!source.eof());
		source.readline(str);
		TS_ASSERT(!source.eof());
		TS_ASSERT_EQUALS(str, "this is a ");
		source.readline(str);
		TS_ASSERT_EQUALS(str, " dos string and a ");
		source.readline(str);
		TS_ASSERT_EQUALS(str, " unix string.");
		TS_ASSERT(source.eof());
		*/
	}

};
