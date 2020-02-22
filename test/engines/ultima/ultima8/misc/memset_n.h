#include <cxxtest/TestSuite.h>
#include "engines/ultima/ultima8/misc/memset_n.h"

/**
 * Test suite for the functions in engines/ultima/ultima8/misc/memset_n.h
 */

class U8MemsetTestSuite : public CxxTest::TestSuite {

	static const uint32 VAL32 = 0xDEADBEEF;
	static const uint32 VAL16 = 0xFEED;

	uint8 buffer[256];

	public:
	U8MemsetTestSuite() {
	}

	static uint32 uint32val(const uint8 *p) {
		return *reinterpret_cast<const uint32 *>(p);
	}

	static uint16 uint16val(const uint8 *p) {
		return *reinterpret_cast<const uint16 *>(p);
	}

	void clear_buffer() {
		memset(buffer, 0, 256);
	}

	void test_memset_32() {
		// Pointer with some padding so we can make sure it does the right
		// thing at start and end of memory block
		uint8 *b = buffer + 16;

		// Check a few alignments to make sure it does the right thing
		// Starting alignment is not important as we cycle through them all
		for (int i = 0; i < 10; i++) {
			b++;
			clear_buffer();
			Ultima::Ultima8::memset_32(b, VAL32, 1);
			TS_ASSERT_EQUALS(uint32val(b), VAL32);
			TS_ASSERT_EQUALS(uint32val(b+4), 0);
			TS_ASSERT_EQUALS(uint32val(b-4), 0);

			clear_buffer();
			Ultima::Ultima8::memset_32(b, VAL32, 2);
			TS_ASSERT_EQUALS(uint32val(b), VAL32);
			TS_ASSERT_EQUALS(uint32val(b+4), VAL32);
			TS_ASSERT_EQUALS(uint32val(b+8), 0);
			TS_ASSERT_EQUALS(uint32val(b-4), 0);
		}
	}

	void test_memset_16() {
		// Pointer with some padding so we can make sure it does the right
		// thing at start and end of memory block
		uint8 *b = buffer + 16;

		// Check a few alignments to make sure it does the right thing
		// Starting alignment is not important as we cycle through them all
		for (int i = 0; i < 10; i++) {
			b++;
			clear_buffer();
			Ultima::Ultima8::memset_16(b, VAL16, 1);
			TS_ASSERT_EQUALS(uint16val(b), VAL16);
			TS_ASSERT_EQUALS(uint16val(b+2), 0);
			TS_ASSERT_EQUALS(uint16val(b-2), 0);

			clear_buffer();
			Ultima::Ultima8::memset_16(b, VAL16, 2);
			TS_ASSERT_EQUALS(uint16val(b), VAL16);
			TS_ASSERT_EQUALS(uint16val(b+2), VAL16);
			TS_ASSERT_EQUALS(uint16val(b+4), 0);
			TS_ASSERT_EQUALS(uint16val(b-2), 0);

			clear_buffer();
			Ultima::Ultima8::memset_16(b, VAL16, 3);
			TS_ASSERT_EQUALS(uint16val(b), VAL16);
			TS_ASSERT_EQUALS(uint16val(b+2), VAL16);
			TS_ASSERT_EQUALS(uint16val(b+4), VAL16);
			TS_ASSERT_EQUALS(uint16val(b+6), 0);
			TS_ASSERT_EQUALS(uint16val(b-2), 0);

			clear_buffer();
			Ultima::Ultima8::memset_16(b, VAL16, 4);
			TS_ASSERT_EQUALS(uint16val(b), VAL16);
			TS_ASSERT_EQUALS(uint16val(b+2), VAL16);
			TS_ASSERT_EQUALS(uint16val(b+4), VAL16);
			TS_ASSERT_EQUALS(uint16val(b+6), VAL16);
			TS_ASSERT_EQUALS(uint16val(b+8), 0);
			TS_ASSERT_EQUALS(uint16val(b-2), 0);
		 }
	}
};
