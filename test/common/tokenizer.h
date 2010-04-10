#include <cxxtest/TestSuite.h>
#include "common/util.h"
#include "common/tokenizer.h"

class TokenizerTestSuite : public CxxTest::TestSuite {
public:
	void test_nextToken() {
		
		// test Common::StringTokenizer class

		// test normal behavior
		Common::StringTokenizer strTokenizer("Now, this is a test!", " ,!");
		Common::String tokenArray[] = {"Now", "this", "is", "a", "test"};

		for (int i = 0; i < ARRAYSIZE(tokenArray); i++ ) {
			// make sure nextToken works correctly
			TS_ASSERT_EQUALS(tokenArray[i], strTokenizer.nextToken());
		}

		// test edgy conditions:
	
		// Empty String
		Common::StringTokenizer s1("");
		TS_ASSERT_EQUALS("", s1.nextToken());
		
		// Empty Delimiter
		Common::StringTokenizer s2("test String", "");
		TS_ASSERT_EQUALS("test String", s2.nextToken());
		
		// String is the delimiter
		Common::StringTokenizer s3("abc", "abc");
		TS_ASSERT_EQUALS("", s3.nextToken());
		// Tokenizer should be empty
		TS_ASSERT(s3.empty());
		
		// consecutive delimiters in the string
		Common::StringTokenizer s4("strstr,after all!!", "str, !");
		TS_ASSERT_EQUALS("af", s4.nextToken());
	}

	void test_resetAndEmpty() {	
		Common::StringTokenizer strTokenizer("Just, another test!", " ,!");

		// test reset()
		Common::String token1 = strTokenizer.nextToken(); //Just
		strTokenizer.reset();
		Common::String token2 = strTokenizer.nextToken(); //Just

		TS_ASSERT_EQUALS(token1,token2);

		// test empty()
		TS_ASSERT(!strTokenizer.empty()); 
		strTokenizer.nextToken(); //another
		strTokenizer.nextToken(); //test
		TS_ASSERT(strTokenizer.empty()); 
	}

};

