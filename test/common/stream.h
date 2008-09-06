#include <cxxtest/TestSuite.h>

#include "common/stream.h"

class ReadLineStreamTestSuite : public CxxTest::TestSuite {
	public:
	void test_readline(void) {
		byte contents[] = { 'a', 'b', '\n', '\n', 'c', '\n' };
		Common::MemoryReadStream ms(contents, sizeof(contents));
		
		char buffer[100];
		
		TS_ASSERT(0 != ms.readLine_NEW(buffer, sizeof(buffer)));
		TS_ASSERT(0 == strcmp(buffer, "ab\n"));

		TS_ASSERT(0 != ms.readLine_NEW(buffer, sizeof(buffer)));
		TS_ASSERT(0 == strcmp(buffer, "\n"));

		TS_ASSERT(0 != ms.readLine_NEW(buffer, sizeof(buffer)));
		TS_ASSERT(0 == strcmp(buffer, "c\n"));

		TS_ASSERT(ms.eos());
	}

	void test_readline2(void) {
		byte contents[] = { 'a', 'b', '\n', '\n', 'c' };
		Common::MemoryReadStream ms(contents, sizeof(contents));
		
		char buffer[100];
		
		TS_ASSERT(0 != ms.readLine_NEW(buffer, sizeof(buffer)));
		TS_ASSERT(0 == strcmp(buffer, "ab\n"));

		TS_ASSERT(0 != ms.readLine_NEW(buffer, sizeof(buffer)));
		TS_ASSERT(0 == strcmp(buffer, "\n"));

		TS_ASSERT(0 != ms.readLine_NEW(buffer, sizeof(buffer)));
		TS_ASSERT(0 == strcmp(buffer, "c"));

		TS_ASSERT(ms.eos());
	}
};
