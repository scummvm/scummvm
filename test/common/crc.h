#include <cxxtest/TestSuite.h>

#include "common/crc.h"

namespace {
const byte *testString = (const byte *)"The quick brown fox jumps over the lazy dog";
const int testLen = 43;
}

class CrcTestSuite : public CxxTest::TestSuite
{
public:
	void test_crc32() {
		Common::CRC32 crc;
		TS_ASSERT_EQUALS(crc.crcFast(testString, testLen), 0x414fa339U);
		TS_ASSERT_EQUALS(crc.crcSlow(testString, testLen), 0x414fa339U);
		uint32 running = crc.getInitRemainder();
		for (const byte *ptr = testString; *ptr; ptr++)
			running = crc.processByte(*ptr, running);
			TS_ASSERT_EQUALS(crc.finalize(running), 0x414fa339U);
	}

	void test_crc16() {
		Common::CRC16 crc;
		TS_ASSERT_EQUALS(crc.crcFast(testString, testLen), 0xfcdfU);
		TS_ASSERT_EQUALS(crc.crcSlow(testString, testLen), 0xfcdfU);
		uint16 running = crc.getInitRemainder();
		for (const byte *ptr = testString; *ptr; ptr++)
			running = crc.processByte(*ptr, running);
			TS_ASSERT_EQUALS(crc.finalize(running), 0xfcdfU);
	}

	void test_crc_ccitt() {
		Common::CRC_CCITT crc; // aka ccitt-false
		TS_ASSERT_EQUALS(crc.crcFast(testString, testLen), 0x8fddU);
		TS_ASSERT_EQUALS(crc.crcSlow(testString, testLen), 0x8fddU);
		uint16 running = crc.getInitRemainder();
		for (const byte *ptr = testString; *ptr; ptr++)
			running = crc.processByte(*ptr, running);
			TS_ASSERT_EQUALS(crc.finalize(running), 0x8fddU);
	}

	void test_crc_binhex() {
		Common::CRC_BINHEX crc; // Aka xmodem
		TS_ASSERT_EQUALS(crc.crcFast(testString, testLen), 0xf0c8U);
		TS_ASSERT_EQUALS(crc.crcSlow(testString, testLen), 0xf0c8U);
		uint16 running = crc.getInitRemainder();
		for (const byte *ptr = testString; *ptr; ptr++)
			running = crc.processByte(*ptr, running);
			TS_ASSERT_EQUALS(crc.finalize(running), 0xf0c8U);
	}
};
