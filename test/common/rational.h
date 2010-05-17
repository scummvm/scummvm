#include <cxxtest/TestSuite.h>

#include "common/rational.h"

class RationalTestSuite : public CxxTest::TestSuite {
public:
	void test_operators() {
		Common::Rational r0(6, 3);
		Common::Rational r1(1, 2);

		Common::Rational r2(62, 2);
		Common::Rational r3(34, 4);

		Common::Rational r4 = (r0 + r1) * 3;
		Common::Rational r5 = (r2 - r3) / 3;

		Common::Rational r6 = r5 - 1;

		TS_ASSERT(r4 == r5);

		TS_ASSERT(-r4 == -r5);

		TS_ASSERT(  r4 >  r6);
		TS_ASSERT(  r4 >= r6);
		TS_ASSERT(!(r4 <  r6));
		TS_ASSERT(!(r4 <= r6));

		TS_ASSERT(  r4 >  7);
		TS_ASSERT(  r4 >= 7);
		TS_ASSERT(!(r4 <  7));
		TS_ASSERT(!(r4 <= 7));

		TS_ASSERT(  7 <  r4);
		TS_ASSERT(  7 <= r4);
		TS_ASSERT(!(7 >  r4));
		TS_ASSERT(!(7 >= r4));
	}
};
