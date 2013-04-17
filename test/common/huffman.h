#include <cxxtest/TestSuite.h>
#include "common/huffman.h"
#include "common/bitstream.h"
#include "common/memstream.h"

class HuffmanTestSuite : public CxxTest::TestSuite
{
	public:
	void test_get_with_full_symbols() {

		/*
		 * Testing the Huffman decoder.
		 *
		 * Encoding (arbitrary, for testing purpouses):
		 * A=010
		 * B=011
		 * C=11
		 * D=00
		 * E=10
		 */

		uint32 codeCount = 5;
		uint8 maxLength = 3;
		const uint8 lengths[] = {3,3,2,2,2};
		const uint32 codes[]  = {0x2, 0x3, 0x3, 0x0, 0x2};
		const uint32 symbols[]  = {0xA, 0xB, 0xC, 0xD, 0xE};

		Common::Huffman h (maxLength, codeCount, codes, lengths, symbols);

		byte input[] = {0x4F, 0x20};
		// Provided input...
		uint32 expected[] = {0xA, 0xB, 0xC, 0xD, 0xE, 0xD, 0xD};
		// ..and expected output.

		/*
		 * What should be going on:
		 * 010 011 11 00 10 00 00 = A B C D E D D
		 *  = 0100 1111 0010 0000 = 0x4F20
		 */

		Common::MemoryReadStream ms(input, sizeof(input));
		Common::BitStream8MSB bs(ms);

		TS_ASSERT_EQUALS(h.getSymbol(bs), expected[0]);
		TS_ASSERT_EQUALS(h.getSymbol(bs), expected[1]);
		TS_ASSERT_EQUALS(h.getSymbol(bs), expected[2]);
		TS_ASSERT_EQUALS(h.getSymbol(bs), expected[3]);
		TS_ASSERT_EQUALS(h.getSymbol(bs), expected[4]);
		TS_ASSERT_EQUALS(h.getSymbol(bs), expected[5]);
		TS_ASSERT_EQUALS(h.getSymbol(bs), expected[6]);
	}

	void test_get_without_symbols() {

		/*
		 * This is basically the same as above, but
		 * I pass minimal arguments.
		 * Specifically, I avoid passing the symbols table, so that
		 * array indices are used instead.
		 *
		 * Encoding becomes:
		 *
		 * 0=010
		 * 1=011
		 * 2=11
		 * 3=00
		 * 4=10
		 */

		uint32 codeCount = 5;
		const uint8 lengths[] = {3,3,2,2,2};
		const uint32 codes[]  = {0x2, 0x3, 0x3, 0x0, 0x2};

		Common::Huffman h (0, codeCount, codes, lengths, 0);

		byte input[] = {0x4F, 0x20};
		uint32 expected[] = {0, 1, 2, 3, 4, 3 ,3};

		Common::MemoryReadStream ms(input, sizeof(input));
		Common::BitStream8MSB bs(ms);

		TS_ASSERT_EQUALS(h.getSymbol(bs), expected[0]);
		TS_ASSERT_EQUALS(h.getSymbol(bs), expected[1]);
		TS_ASSERT_EQUALS(h.getSymbol(bs), expected[2]);
		TS_ASSERT_EQUALS(h.getSymbol(bs), expected[3]);
		TS_ASSERT_EQUALS(h.getSymbol(bs), expected[4]);
		TS_ASSERT_EQUALS(h.getSymbol(bs), expected[5]);
		TS_ASSERT_EQUALS(h.getSymbol(bs), expected[6]);
	}

	void test_get_after_set_symbols() {

		/*
		 * Another variation of the above.
		 * I use the setSymbols method to define, a posteriori,
		 * an alphabet to be used in place of array indices
		 */

		uint32 codeCount = 5;
		const uint8 lengths[] = {3,3,2,2,2};
		const uint32 codes[]  = {0x2, 0x3, 0x3, 0x0, 0x2};

		Common::Huffman h (0, codeCount, codes, lengths, 0);

		const uint32 symbols[]  = {0xA, 0xB, 0xC, 0xD, 0xE};
		h.setSymbols(symbols);

		byte input[] = {0x4F, 0x20};
		uint32 expected[] = {0xA, 0xB, 0xC, 0xD, 0xE, 0xD, 0xD};

		Common::MemoryReadStream ms(input, sizeof(input));
		Common::BitStream8MSB bs(ms);

		/* New symbols:
		 * A=010
		 * B=011
		 * C=11
		 * D=00
		 * E=10
		 */

		TS_ASSERT_EQUALS(h.getSymbol(bs), expected[0]);
		TS_ASSERT_EQUALS(h.getSymbol(bs), expected[1]);
		TS_ASSERT_EQUALS(h.getSymbol(bs), expected[2]);
		TS_ASSERT_EQUALS(h.getSymbol(bs), expected[3]);
		TS_ASSERT_EQUALS(h.getSymbol(bs), expected[4]);
		TS_ASSERT_EQUALS(h.getSymbol(bs), expected[5]);
		TS_ASSERT_EQUALS(h.getSymbol(bs), expected[6]);
	}
};
