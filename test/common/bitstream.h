#include <cxxtest/TestSuite.h>

#include "common/bitstream.h"
#include "common/memstream.h"

class BitStreamTestSuite : public CxxTest::TestSuite
{
private:
	template<class MS, class BS>
	void tmpl_get_bit() {
		byte contents[] = { 'a' };

		MS ms(contents, sizeof(contents));

		BS bs(ms);
		TS_ASSERT_EQUALS(bs.pos(), 0u);
		TS_ASSERT_EQUALS(bs.getBit(), 0u);
		TS_ASSERT_EQUALS(bs.getBit(), 1u);
		TS_ASSERT_EQUALS(bs.getBit(), 1u);
		TS_ASSERT_EQUALS(bs.pos(), 3u);
		TS_ASSERT(!bs.eos());
	}
public:
	void test_get_bit() {
		tmpl_get_bit<Common::MemoryReadStream, Common::BitStream8MSB>();
		tmpl_get_bit<Common::BitStreamMemoryStream, Common::BitStreamMemory8MSB>();
	}

private:
	template<class MS, class BS>
	void tmpl_get_bits() {
		byte contents[] = { 'a', 'b' };

		MS ms(contents, sizeof(contents));

		BS bs(ms);
		TS_ASSERT_EQUALS(bs.pos(), 0u);
		TS_ASSERT_EQUALS(bs.getBits(3), 3u);
		TS_ASSERT_EQUALS(bs.pos(), 3u);
		TS_ASSERT_EQUALS(bs.getBits(8), 11u);
		TS_ASSERT_EQUALS(bs.pos(), 11u);
		TS_ASSERT(!bs.eos());
	}
public:
	void test_get_bits() {
		tmpl_get_bits<Common::MemoryReadStream, Common::BitStream8MSB>();
		tmpl_get_bits<Common::BitStreamMemoryStream, Common::BitStreamMemory8MSB>();
	}

private:
	template<class MS, class BS>
	void tmpl_skip() {
		byte contents[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j' };

		MS ms(contents, sizeof(contents));

		BS bs(ms);
		TS_ASSERT_EQUALS(bs.pos(), 0u);
		bs.skip(5);
		TS_ASSERT_EQUALS(bs.pos(), 5u);
		bs.skip(4);
		TS_ASSERT_EQUALS(bs.pos(), 9u);
		TS_ASSERT_EQUALS(bs.getBits(3), 6u);
		bs.skip(65);
		TS_ASSERT_EQUALS(bs.pos(), 77u);
		TS_ASSERT(!bs.eos());
	}
public:
	void test_skip() {
		tmpl_skip<Common::MemoryReadStream, Common::BitStream8MSB>();
		tmpl_skip<Common::BitStreamMemoryStream, Common::BitStreamMemory8MSB>();
	}

private:
	template<class MS, class BS>
	void tmpl_rewind() {
		byte contents[] = { 'a' };

		MS ms(contents, sizeof(contents));

		BS bs(ms);
		TS_ASSERT_EQUALS(bs.pos(), 0u);
		bs.skip(5);
		TS_ASSERT_EQUALS(bs.pos(), 5u);
		bs.rewind();
		TS_ASSERT_EQUALS(bs.pos(), 0u);
		TS_ASSERT_EQUALS(bs.getBits(3), 3u);
		TS_ASSERT(!bs.eos());

		TS_ASSERT_EQUALS(bs.size(), 8u);
	}
public:
	void test_rewind() {
		tmpl_rewind<Common::MemoryReadStream, Common::BitStream8MSB>();
		tmpl_rewind<Common::BitStreamMemoryStream, Common::BitStreamMemory8MSB>();
	}

private:
	template<class MS, class BS>
	void tmpl_peek_bit() {
		byte contents[] = { 'a' };

		MS ms(contents, sizeof(contents));

		BS bs(ms);
		TS_ASSERT_EQUALS(bs.pos(), 0u);
		TS_ASSERT_EQUALS(bs.peekBit(), 0u);
		TS_ASSERT_EQUALS(bs.pos(), 0u);
		TS_ASSERT_EQUALS(bs.getBit(), 0u);
		TS_ASSERT_EQUALS(bs.pos(), 1u);
		TS_ASSERT_EQUALS(bs.peekBit(), 1u);
		TS_ASSERT_EQUALS(bs.pos(), 1u);
		TS_ASSERT(!bs.eos());
	}
public:
	void test_peek_bit() {
		tmpl_peek_bit<Common::MemoryReadStream, Common::BitStream8MSB>();
		tmpl_peek_bit<Common::BitStreamMemoryStream, Common::BitStreamMemory8MSB>();
	}

private:
	template<class MS, class BS>
	void tmpl_peek_bits() {
		byte contents[] = { 'a', 'b' };

		MS ms(contents, sizeof(contents));

		BS bs(ms);
		TS_ASSERT_EQUALS(bs.pos(), 0u);
		TS_ASSERT_EQUALS(bs.peekBits(3), 3u);
		TS_ASSERT_EQUALS(bs.pos(), 0u);
		bs.skip(3);
		TS_ASSERT_EQUALS(bs.pos(), 3u);
		TS_ASSERT_EQUALS(bs.peekBits(8), 11u);
		TS_ASSERT_EQUALS(bs.pos(), 3u);
		bs.skip(8);
		TS_ASSERT_EQUALS(bs.pos(), 11u);
		TS_ASSERT_EQUALS(bs.peekBits(6), 4u);
		TS_ASSERT(!bs.eos());
	}
public:
	void test_peek_bits() {
		tmpl_peek_bits<Common::MemoryReadStream, Common::BitStream8MSB>();
		tmpl_peek_bits<Common::BitStreamMemoryStream, Common::BitStreamMemory8MSB>();
	}

private:
	template<class MS, class BS>
	void tmpl_eos() {
		byte contents[] = { 'a', 'b' };

		MS ms(contents, sizeof(contents));

		BS bs(ms);
		bs.skip(11);
		TS_ASSERT_EQUALS(bs.pos(), 11u);
		TS_ASSERT_EQUALS(bs.getBits(5), 2u);
		TS_ASSERT(bs.eos());

		bs.rewind();
		TS_ASSERT_EQUALS(bs.pos(), 0u);
		TS_ASSERT(!bs.eos());
	}
public:
	void test_eos() {
		tmpl_eos<Common::MemoryReadStream, Common::BitStream8MSB>();
		tmpl_eos<Common::BitStreamMemoryStream, Common::BitStreamMemory8MSB>();
	}

private:
	template<class MS, class BS>
	void tmpl_get_bits_lsb() {
		byte contents[] = { 'a', 'b' };

		MS ms(contents, sizeof(contents));

		BS bs(ms);
		TS_ASSERT_EQUALS(bs.pos(), 0u);
		TS_ASSERT_EQUALS(bs.getBits(3), 1u);
		TS_ASSERT_EQUALS(bs.pos(), 3u);
		TS_ASSERT_EQUALS(bs.getBits(8), 76u);
		TS_ASSERT_EQUALS(bs.pos(), 11u);
		TS_ASSERT(!bs.eos());
	}
public:
	void test_get_bits_lsb() {
		tmpl_get_bits_lsb<Common::MemoryReadStream, Common::BitStream8LSB>();
		tmpl_get_bits_lsb<Common::BitStreamMemoryStream, Common::BitStreamMemory8LSB>();
	}

private:
	template<class MS, class BS>
	void tmpl_peek_bits_lsb() {
		byte contents[] = { 'a', 'b' };

		MS ms(contents, sizeof(contents));

		BS bs(ms);
		TS_ASSERT_EQUALS(bs.pos(), 0u);
		TS_ASSERT_EQUALS(bs.peekBits(3), 1u);
		TS_ASSERT_EQUALS(bs.pos(), 0u);
		bs.skip(3);
		TS_ASSERT_EQUALS(bs.pos(), 3u);
		TS_ASSERT_EQUALS(bs.peekBits(8), 76u);
		TS_ASSERT_EQUALS(bs.pos(), 3u);
		bs.skip(8);
		TS_ASSERT_EQUALS(bs.pos(), 11u);
		TS_ASSERT_EQUALS(bs.peekBits(20), 12u);
		TS_ASSERT(!bs.eos());
	}
public:
	void test_peek_bits_lsb() {
		tmpl_peek_bits_lsb<Common::MemoryReadStream, Common::BitStream8LSB>();
		tmpl_peek_bits_lsb<Common::BitStreamMemoryStream, Common::BitStreamMemory8LSB>();
	}

private:
	template<class MS, class BS>
	void tmpl_align() {
		byte contents[] = { 'a', 'b' };

		MS ms(contents, sizeof(contents));

		BS bs(ms);
		TS_ASSERT_EQUALS(bs.pos(), 0u);
		bs.align();
		TS_ASSERT_EQUALS(bs.pos(), 0u);
		bs.skip(3);
		bs.align();
		TS_ASSERT_EQUALS(bs.pos(), 8u);
	}
public:
	void test_align() {
		tmpl_align<Common::MemoryReadStream, Common::BitStream8LSB>();
		tmpl_align<Common::BitStreamMemoryStream, Common::BitStreamMemory8LSB>();
	}

private:
	template<class MS, class BS>
	void tmpl_align_16() {
		byte contents[] = { 'a', 'b' };

		MS ms(contents, sizeof(contents));

		BS bs(ms);
		TS_ASSERT_EQUALS(bs.pos(), 0u);
		bs.align();
		TS_ASSERT_EQUALS(bs.pos(), 0u);
		bs.skip(3);
		bs.align();
		TS_ASSERT_EQUALS(bs.pos(), 16u);
	}
public:
	void test_align_16() {
		tmpl_align_16<Common::MemoryReadStream, Common::BitStream16BELSB>();
		tmpl_align_16<Common::BitStreamMemoryStream, Common::BitStreamMemory16BELSB>();
	}
};
