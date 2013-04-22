#include <cxxtest/TestSuite.h>
#include "common/cosinetables.h"

/**
* Some test cases for the CosineTable class.
* This is SineTable's twin, so there is some duplication.
*/
class CosineTableTestSuite : public CxxTest::TestSuite {

	static const int kMinBitPrecision = 4;
	static const int kMaxBitPrecision = 16;

	// Precision is, as a precondition, expected to
	// range between 4 and 16.
	// We do not test above and below.

	public:
	void test_for_precision() {
		// See that precision is correctly held.
		for (int p = kMinBitPrecision; p <= kMaxBitPrecision; p++) {
			Common::CosineTable ct(p);
			TS_ASSERT(ct.getPrecision() == p);
		}
	}

	void test_common_angles() {

		// Test for common angles.
		// This at the moment means cos(0)==1

		for (int p = kMinBitPrecision; p <= kMaxBitPrecision; p++) {
			Common::CosineTable ct(p);
			float *table = const_cast <float*> (ct.getTable());
			TS_ASSERT(table[0] == 1);
		}
	}

	void test_for_symmetry() {

		// The table is, by definition of the cos function,
		// expected to be symmetric around its 2^(p-2)-th point
		// in the absence of rounding errors.

		for (int p = kMinBitPrecision; p <= kMaxBitPrecision; p++) {
			int entries = 1;
			for (int exp = 0; exp < (p - 1); exp++) {
				entries = entries * 2;
			}
			// After this, entries = 2^^(p-1)
			int half_entries = entries / 2;
			Common::CosineTable ct(p);
			float *table = const_cast <float*> (ct.getTable());
			for (int i = 1; i < half_entries; i++) {
				TS_ASSERT(table[i] == table[entries - i]);
			}
		}
	}
};
