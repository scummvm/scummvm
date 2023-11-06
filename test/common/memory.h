#include <cxxtest/TestSuite.h>

#include "common/memory.h"

class MemoryTestSuite : public CxxTest::TestSuite {
public:
	void test_memset16() {
		uint16    expected[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		const uint16 step1[8] = { 0, 1, 1, 1, 1, 1, 1, 0 };
		const uint16 step2[8] = { 0, 1, 2, 2, 2, 2, 1, 0 };
		const uint16 step3[8] = { 0, 1, 2, 3, 3, 2, 1, 0 };

		Common::memset16(expected + 1, 1, 6);

		TS_ASSERT_EQUALS(memcmp(expected, step1, sizeof(expected)), 0);

		Common::memset16(expected + 2, 2, 4);

		TS_ASSERT_EQUALS(memcmp(expected, step2, sizeof(expected)), 0);

		Common::memset16(expected + 3, 3, 2);

		TS_ASSERT_EQUALS(memcmp(expected, step3, sizeof(expected)), 0);
	}

	void test_memset32() {
		uint32    expected[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		const uint32 step1[8] = { 0, 1, 1, 1, 1, 1, 1, 0 };
		const uint32 step2[8] = { 0, 1, 2, 2, 2, 2, 1, 0 };
		const uint32 step3[8] = { 0, 1, 2, 3, 3, 2, 1, 0 };

		Common::memset32(expected + 1, 1, 6);

		TS_ASSERT_EQUALS(memcmp(expected, step1, sizeof(expected)), 0);

		Common::memset32(expected + 2, 2, 4);

		TS_ASSERT_EQUALS(memcmp(expected, step2, sizeof(expected)), 0);

		Common::memset32(expected + 3, 3, 2);

		TS_ASSERT_EQUALS(memcmp(expected, step3, sizeof(expected)), 0);
	}

	void test_memset64() {
		uint64    expected[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		const uint64 step1[8] = { 0, 1, 1, 1, 1, 1, 1, 0 };
		const uint64 step2[8] = { 0, 1, 2, 2, 2, 2, 1, 0 };
		const uint64 step3[8] = { 0, 1, 2, 3, 3, 2, 1, 0 };

		Common::memset64(expected + 1, 1, 6);

		TS_ASSERT_EQUALS(memcmp(expected, step1, sizeof(expected)), 0);

		Common::memset64(expected + 2, 2, 4);

		TS_ASSERT_EQUALS(memcmp(expected, step2, sizeof(expected)), 0);

		Common::memset64(expected + 3, 3, 2);

		TS_ASSERT_EQUALS(memcmp(expected, step3, sizeof(expected)), 0);
	}
};
