#include <cxxtest/TestSuite.h>

#include "sound/timestamp.h"

class TimestampTestSuite : public CxxTest::TestSuite
{
	public:
	void test_diff_add(void) {
		Audio::Timestamp a(10000, 1000);
		Audio::Timestamp b(10001, 1000);
		Audio::Timestamp c(10002, 1000);
	
		TS_ASSERT_EQUALS(a.frameDiff(b), -1);
		TS_ASSERT_EQUALS(b.frameDiff(a), 1);
		TS_ASSERT_EQUALS(c.frameDiff(a), 2);
		TS_ASSERT_EQUALS(b.addFrames(2000).frameDiff(a), 2001);
		TS_ASSERT_EQUALS(a.frameDiff(b), -1);
		TS_ASSERT_EQUALS(b.frameDiff(a), 1);
		TS_ASSERT_EQUALS(c.frameDiff(a), 2);
		TS_ASSERT_EQUALS(b.addFrames(2000).frameDiff(a.addFrames(-1000)), 3001);
		TS_ASSERT_EQUALS(a.frameDiff(b), -1);
		TS_ASSERT_EQUALS(b.frameDiff(a), 1);
		TS_ASSERT_EQUALS(c.frameDiff(a), 2);
	}

	void test_more_add_diff(void) {
		const Audio::Timestamp c(10002, 1000);

		for (int i = -10000; i < 10000; i++) {
			int v = c.addFrames(i).frameDiff(c);
			TS_ASSERT_EQUALS(v, i);
		}
	}
};
