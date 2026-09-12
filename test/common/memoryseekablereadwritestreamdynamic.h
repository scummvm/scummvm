#include <cxxtest/TestSuite.h>

#include "common/memstream.h"

class MemorySeekableReadWriteStreamDynamicTestSuite : public CxxTest::TestSuite {
public:
	void testReadWriteAndSeek() {
		Common::MemorySeekableReadWriteStreamDynamic stream(DisposeAfterUse::YES);
		const char initial[] = "abcdef";
		TS_ASSERT_EQUALS(stream.write(initial, 6), 6U);

		TS_ASSERT(stream.seek(2));
		const char replacement[] = "XY";
		TS_ASSERT_EQUALS(stream.write(replacement, 2), 2U);
		TS_ASSERT(stream.seek(-3, SEEK_END));
		TS_ASSERT(stream.seek(-1, SEEK_CUR));

		char result[5] = {};
		TS_ASSERT_EQUALS(stream.read(result, 4), 4U);
		TS_ASSERT_EQUALS(memcmp(result, "XYef", 4), 0);
	}

	void testGrowthAndReread() {
		Common::MemorySeekableReadWriteStreamDynamic stream(DisposeAfterUse::YES);
		const char initial[] = "abc";
		TS_ASSERT_EQUALS(stream.write(initial, 3), 3U);
		const char extension[] = "defghijklmnopqrstuvwxyz";
		TS_ASSERT_EQUALS(stream.write(extension, 23), 23U);
		TS_ASSERT_EQUALS(stream.size(), 26);

		TS_ASSERT(stream.seek(0));
		char result[27] = {};
		TS_ASSERT_EQUALS(stream.read(result, 26), 26U);
		TS_ASSERT_EQUALS(memcmp(result, "abcdefghijklmnopqrstuvwxyz", 26), 0);

		TS_ASSERT(stream.seek(0));
		TS_ASSERT_EQUALS(stream.read(result, 26), 26U);
		TS_ASSERT_EQUALS(memcmp(result, "abcdefghijklmnopqrstuvwxyz", 26), 0);
	}
};
