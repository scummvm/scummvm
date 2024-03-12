#include <cxxtest/TestSuite.h>

#include "common/crc.h"
#include "common/crc_slow.h"

namespace {
const byte *testStringCRC = (const byte *)"The quick brown fox jumps over the lazy dog";
const int testLenCRC = 43;
}

class CrcTestSuite : public CxxTest::TestSuite
{
public:
	void test_crc32() {
		Common::CRC32 crc;
		TS_ASSERT_EQUALS(crc.crcFast(testStringCRC, testLenCRC), 0x414fa339U);
		uint32 running = crc.getInitRemainder();
		for (const byte *ptr = testStringCRC; *ptr; ptr++) {
			running = crc.processByte(*ptr, running);
		}
		TS_ASSERT_EQUALS(crc.finalize(running), 0x414fa339U);
	}

	void test_crc16() {
		Common::CRC16 crc;
		TS_ASSERT_EQUALS(crc.crcFast(testStringCRC, testLenCRC), 0xfcdfU);
		uint16 running = crc.getInitRemainder();
		for (const byte *ptr = testStringCRC; *ptr; ptr++) {
			running = crc.processByte(*ptr, running);
		}
		TS_ASSERT_EQUALS(crc.finalize(running), 0xfcdfU);
	}

	void test_crc_ccitt() {
		Common::CRC_CCITT crc; // aka ccitt-false
		TS_ASSERT_EQUALS(crc.crcFast(testStringCRC, testLenCRC), 0x8fddU);
		uint16 running = crc.getInitRemainder();
		for (const byte *ptr = testStringCRC; *ptr; ptr++) {
			running = crc.processByte(*ptr, running);
		}
		TS_ASSERT_EQUALS(crc.finalize(running), 0x8fddU);
	}

	void test_crc_binhex() {
		Common::CRC_BINHEX crc; // Aka xmodem
		TS_ASSERT_EQUALS(crc.crcFast(testStringCRC, testLenCRC), 0xf0c8U);
		uint16 running = crc.getInitRemainder();
		for (const byte *ptr = testStringCRC; *ptr; ptr++) {
			running = crc.processByte(*ptr, running);
		}
		TS_ASSERT_EQUALS(crc.finalize(running), 0xf0c8U);
	}

	void test_crc32_slow() {
		Common::CRC32_Slow crc;
		TS_ASSERT_EQUALS(crc.crcSlow(testStringCRC, testLenCRC), 0x414fa339U);
	}

	void test_crc16_slow() {
		Common::CRC16_Slow crc;
		TS_ASSERT_EQUALS(crc.crcSlow(testStringCRC, testLenCRC), 0xfcdfU);
	}

	void test_crc_ccitt_slow() {
		Common::CRC_CCITT_Slow crc; // aka ccitt-false
		TS_ASSERT_EQUALS(crc.crcSlow(testStringCRC, testLenCRC), 0x8fddU);
	}

	void test_crc_binhex_slow() {
		Common::CRC_BINHEX_Slow crc; // Aka xmodem
		TS_ASSERT_EQUALS(crc.crcSlow(testStringCRC, testLenCRC), 0xf0c8U);
	}
};
