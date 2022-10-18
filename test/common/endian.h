#include <cxxtest/TestSuite.h>
#include "common/endian.h"

class EndianTestSuite : public CxxTest::TestSuite
{
	public:
	void test_MKTAG() {
		const char *str_tag = "ABCD";
		uint32 tag = READ_BE_UINT32(str_tag);
		TS_ASSERT_EQUALS(MKTAG('A','B','C','D'), tag);
	}

	void test_READ_BE_UINT64() {
		const byte data[8] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xFF};
		uint64 value = READ_BE_UINT64(data);
		TS_ASSERT_EQUALS(value, 0x123456789ABCDEFFULL);
	}

	void test_READ_LE_UINT64() {
		const byte data[8] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xFF};
		uint64 value = READ_LE_UINT64(data);
		TS_ASSERT_EQUALS(value, 0xFFDEBC9A78563412ULL);
	}

	void test_READ_BE_UINT32() {
		const char data[4] = { 0x12, 0x34, 0x56, 0x78 };
		uint32 value = READ_BE_UINT32(data);
		TS_ASSERT_EQUALS(value, 0x12345678UL);
	}

	void test_READ_LE_UINT32() {
		const char data[4] = { 0x12, 0x34, 0x56, 0x78 };
		uint32 value = READ_LE_UINT32(data);
		TS_ASSERT_EQUALS(value, 0x78563412UL);
	}

	void test_READ_BE_UINT16() {
		const char data[4] = { 0x12, 0x34, 0x56, 0x78 };
		uint32 value = READ_BE_UINT16(data);
		TS_ASSERT_EQUALS(value, 0x1234UL);
	}

	void test_READ_LE_UINT16() {
		const char data[4] = { 0x12, 0x34, 0x56, 0x78 };
		uint32 value = READ_LE_UINT16(data);
		TS_ASSERT_EQUALS(value, 0x3412UL);
	}

	void test_READ_BE_FLOAT32() {
		const uint8 data[4] = { 0x40, 0x49, 0x0f, 0xdc };
		float value = READ_BE_FLOAT32(data);
		TS_ASSERT_EQUALS(value, 3.141593f);
	}

	void test_READ_LE_FLOAT32() {
		const uint8 data[4] = { 0xdc, 0x0f, 0x49, 0x40 };
		float value = READ_LE_FLOAT32(data);
		TS_ASSERT_EQUALS(value, 3.141593f);
	}

	void test_READ_BE_FLOAT64() {
		const uint8 data[8] = { 0x40, 0x09, 0x21, 0xfb, 0x82, 0xc2, 0xbd, 0x7f };
		double value = READ_BE_FLOAT64(data);
		TS_ASSERT_EQUALS(value, 3.141593);
	}

	void test_READ_LE_FLOAT64() {
		const uint8 data[8] = { 0x7f, 0xbd, 0xc2, 0x82, 0xfb, 0x21, 0x09, 0x40 };
		double value = READ_LE_FLOAT64(data);
		TS_ASSERT_EQUALS(value, 3.141593);
	}

	void test_READ_FPA_FLOAT64() {
		const uint8 data[8] = { 0xfb, 0x21, 0x09, 0x40, 0x7f, 0xbd, 0xc2, 0x82 };
		double value = READ_FPA_FLOAT64(data);
		TS_ASSERT_EQUALS(value, 3.141593);
	}

	void test_WRITE_BE_FLOAT32() {
		const uint8 data[4] = { 0x40, 0x49, 0x0f, 0xdc };
		uint8 out[4];
		WRITE_BE_FLOAT32(out, 3.141593f);
		TS_ASSERT_EQUALS(memcmp(data, out, 4), 0);
	}

	void test_WRITE_LE_FLOAT32() {
		const uint8 data[4] = { 0xdc, 0x0f, 0x49, 0x40 };
		uint8 out[4];
		WRITE_LE_FLOAT32(out, 3.141593f);
		TS_ASSERT_EQUALS(memcmp(data, out, 4), 0);
	}

	void test_WRITE_BE_FLOAT64() {
		const uint8 data[8] = { 0x40, 0x09, 0x21, 0xfb, 0x82, 0xc2, 0xbd, 0x7f };
		uint8 out[8];
		WRITE_BE_FLOAT64(out, 3.141593);
		TS_ASSERT_EQUALS(memcmp(data, out, 8), 0);
	}

	void test_WRITE_LE_FLOAT64() {
		const uint8 data[8] = { 0x7f, 0xbd, 0xc2, 0x82, 0xfb, 0x21, 0x09, 0x40 };
		uint8 out[8];
		WRITE_LE_FLOAT64(out, 3.141593);
		TS_ASSERT_EQUALS(memcmp(data, out, 8), 0);
	}

	void test_WRITE_FPA_FLOAT64() {
		const uint8 data[8] = { 0xfb, 0x21, 0x09, 0x40, 0x7f, 0xbd, 0xc2, 0x82 };
		uint8 out[8];
		WRITE_FPA_FLOAT64(out, 3.141593);
		TS_ASSERT_EQUALS(memcmp(data, out, 8), 0);
	}

};
