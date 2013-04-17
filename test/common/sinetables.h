#include <cxxtest/TestSuite.h>
#include "common/sinetables.h"

class SineTableTestSuite : public CxxTest::TestSuite
{
	static const int minBitPrecision = 4;
	static const int maxBitPrecision = 16;

	public:
	void test_for_precision() {
		// See that precision is correctly mantained
		for (int p=minBitPrecision; p<=maxBitPrecision; p++) {
			Common::SineTable st(p);
			TS_ASSERT(st.getPrecision()==p);
		}
	}

	void test_common_angles() {
		// Test for common angles - at the moment, sin(0)==0
		for (int p=minBitPrecision; p<=maxBitPrecision; p++) {
			Common::SineTable st(p);
			float *table = const_cast <float*> (st.getTable());
			TS_ASSERT(table[0]==0);
		}
	}

	void test_for_symmetry() {
		// The table is expected to be symmetric
		// around its 2^(p-2)-th point
		for (int p=minBitPrecision; p<=maxBitPrecision; p++) {
			int entries=1;
			for (int exp=0; exp<(p-1); exp++) {
				entries = entries * 2;
			}
			// And now entries = 2^^(p-1)
			int half_entries = entries / 2;
			Common::SineTable st(p);
			float *table = const_cast <float*> (st.getTable());
			for (int i=1; i<half_entries; i++){
				TS_ASSERT(table[i]==table[entries-i]);
			}
		}
	}
};
