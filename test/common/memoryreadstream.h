#include <cxxtest/TestSuite.h>

#include "common/stream.h"

class MemoryReadStreamTestSuite : public CxxTest::TestSuite {
	public:
	void test_seek_set() {
		byte contents[] = { 'a', 'b', '\n', '\n', 'c', '\n' };
		Common::MemoryReadStream ms(contents, sizeof(contents));

		ms.seek(0, SEEK_SET);
		TS_ASSERT_EQUALS(ms.pos(), 0);
		TS_ASSERT(!ms.eos());

		ms.seek(1, SEEK_SET);
		TS_ASSERT_EQUALS(ms.pos(), 1);
		TS_ASSERT(!ms.eos());

		ms.seek(5, SEEK_SET);
		TS_ASSERT_EQUALS(ms.pos(), 5);
		TS_ASSERT(!ms.eos());
	}

	void test_seek_cur() {
		byte contents[] = { 'a', 'b', '\n', '\n', 'c' };
		Common::MemoryReadStream ms(contents, sizeof(contents));

		ms.seek(3, SEEK_CUR);
		TS_ASSERT_EQUALS(ms.pos(), 3);
		TS_ASSERT(!ms.eos());

		ms.seek(-1, SEEK_CUR);
		TS_ASSERT_EQUALS(ms.pos(), 2);
		TS_ASSERT(!ms.eos());

		ms.seek(3, SEEK_CUR);
		TS_ASSERT_EQUALS(ms.pos(), 5);
		TS_ASSERT(!ms.eos());

		ms.seek(-1, SEEK_CUR);
		TS_ASSERT_EQUALS(ms.pos(), 4);
		TS_ASSERT(!ms.eos());
	}

	void test_seek_end() {
		byte contents[] = { 'a', 'b', '\n', '\n', 'c' };
		Common::MemoryReadStream ms(contents, sizeof(contents));

		ms.seek(0, SEEK_END);
		TS_ASSERT_EQUALS(ms.pos(), 5);
		TS_ASSERT(!ms.eos());

		ms.seek(-1, SEEK_END);
		TS_ASSERT_EQUALS(ms.pos(), 4);
		TS_ASSERT(!ms.eos());

		ms.seek(-5, SEEK_END);
		TS_ASSERT_EQUALS(ms.pos(), 0);
		TS_ASSERT(!ms.eos());
	}
};
