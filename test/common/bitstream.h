#include <cxxtest/TestSuite.h>

#include "common/bitstream.h"
#include "common/memstream.h"

class BitStreamTestSuite : public CxxTest::TestSuite
{
	public:
	void test_get_bit() {
		byte contents[] = { 'a' };

		Common::MemoryReadStream ms(contents, sizeof(contents));

		Common::BitStream8MSB bs(ms);
		TS_ASSERT_EQUALS(bs.pos(), 0u);
		TS_ASSERT_EQUALS(bs.getBit(), 0u);
		TS_ASSERT_EQUALS(bs.getBit(), 1u);
		TS_ASSERT_EQUALS(bs.getBit(), 1u);
		TS_ASSERT_EQUALS(bs.pos(), 3u);
		TS_ASSERT(!bs.eos());
	}

	void test_get_bits() {
		byte contents[] = { 'a', 'b' };

		Common::MemoryReadStream ms(contents, sizeof(contents));

		Common::BitStream8MSB bs(ms);
		TS_ASSERT_EQUALS(bs.pos(), 0u);
		TS_ASSERT_EQUALS(bs.getBits(3), 3u);
		TS_ASSERT_EQUALS(bs.pos(), 3u);
		TS_ASSERT_EQUALS(bs.getBits(8), 11u);
		TS_ASSERT_EQUALS(bs.pos(), 11u);
		TS_ASSERT(!bs.eos());
	}

	void test_skip() {
		byte contents[] = { 'a', 'b' };

		Common::MemoryReadStream ms(contents, sizeof(contents));

		Common::BitStream8MSB bs(ms);
		TS_ASSERT_EQUALS(bs.pos(), 0u);
		bs.skip(5);
		TS_ASSERT_EQUALS(bs.pos(), 5u);
		bs.skip(4);
		TS_ASSERT_EQUALS(bs.pos(), 9u);
		TS_ASSERT_EQUALS(bs.getBits(3), 6u);
		TS_ASSERT(!bs.eos());
	}

	void test_rewind() {
		byte contents[] = { 'a' };

		Common::MemoryReadStream ms(contents, sizeof(contents));

		Common::BitStream8MSB bs(ms);
		TS_ASSERT_EQUALS(bs.pos(), 0u);
		bs.skip(5);
		TS_ASSERT_EQUALS(bs.pos(), 5u);
		bs.rewind();
		TS_ASSERT_EQUALS(bs.pos(), 0u);
		TS_ASSERT_EQUALS(bs.getBits(3), 3u);
		TS_ASSERT(!bs.eos());

		TS_ASSERT_EQUALS(bs.size(), 8u);
	}

	void test_peek_bit() {
		byte contents[] = { 'a' };

		Common::MemoryReadStream ms(contents, sizeof(contents));

		Common::BitStream8MSB bs(ms);
		TS_ASSERT_EQUALS(bs.pos(), 0u);
		TS_ASSERT_EQUALS(bs.peekBit(), 0u);
		TS_ASSERT_EQUALS(bs.pos(), 0u);
		TS_ASSERT_EQUALS(bs.getBit(), 0u);
		TS_ASSERT_EQUALS(bs.pos(), 1u);
		TS_ASSERT_EQUALS(bs.peekBit(), 1u);
		TS_ASSERT_EQUALS(bs.pos(), 1u);
		TS_ASSERT(!bs.eos());
	}

	void test_peek_bits() {
		byte contents[] = { 'a', 'b' };

		Common::MemoryReadStream ms(contents, sizeof(contents));

		Common::BitStream8MSB bs(ms);
		TS_ASSERT_EQUALS(bs.pos(), 0u);
		TS_ASSERT_EQUALS(bs.peekBits(3), 3u);
		TS_ASSERT_EQUALS(bs.pos(), 0u);
		bs.skip(3);
		TS_ASSERT_EQUALS(bs.pos(), 3u);
		TS_ASSERT_EQUALS(bs.peekBits(8), 11u);
		TS_ASSERT_EQUALS(bs.pos(), 3u);
		bs.skip(8);
		TS_ASSERT_EQUALS(bs.pos(), 11u);
		TS_ASSERT_EQUALS(bs.peekBits(5), 2u);
		TS_ASSERT(!bs.eos());
	}

	void test_eos() {
		byte contents[] = { 'a', 'b' };

		Common::MemoryReadStream ms(contents, sizeof(contents));

		Common::BitStream8MSB bs(ms);
		bs.skip(11);
		TS_ASSERT_EQUALS(bs.pos(), 11u);
		TS_ASSERT_EQUALS(bs.getBits(5), 2u);
		TS_ASSERT(bs.eos());

		bs.rewind();
		TS_ASSERT_EQUALS(bs.pos(), 0u);
		TS_ASSERT(!bs.eos());
	}

	void test_get_bits_lsb() {
		byte contents[] = { 'a', 'b' };

		Common::MemoryReadStream ms(contents, sizeof(contents));

		Common::BitStream8LSB bs(ms);
		TS_ASSERT_EQUALS(bs.pos(), 0u);
		TS_ASSERT_EQUALS(bs.getBits(3), 1u);
		TS_ASSERT_EQUALS(bs.pos(), 3u);
		TS_ASSERT_EQUALS(bs.getBits(8), 76u);
		TS_ASSERT_EQUALS(bs.pos(), 11u);
		TS_ASSERT(!bs.eos());
	}

	void test_peek_bits_lsb() {
		byte contents[] = { 'a', 'b' };

		Common::MemoryReadStream ms(contents, sizeof(contents));

		Common::BitStream8LSB bs(ms);
		TS_ASSERT_EQUALS(bs.pos(), 0u);
		TS_ASSERT_EQUALS(bs.peekBits(3), 1u);
		TS_ASSERT_EQUALS(bs.pos(), 0u);
		bs.skip(3);
		TS_ASSERT_EQUALS(bs.pos(), 3u);
		TS_ASSERT_EQUALS(bs.peekBits(8), 76u);
		TS_ASSERT_EQUALS(bs.pos(), 3u);
		bs.skip(8);
		TS_ASSERT_EQUALS(bs.pos(), 11u);
		TS_ASSERT_EQUALS(bs.peekBits(5), 12u);
		TS_ASSERT(!bs.eos());
	}
};
