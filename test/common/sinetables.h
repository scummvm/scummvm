#include <cxxtest/TestSuite.h>
#include "common/sinetables.h"

/**
 * Some test cases for the SineTable class.
 * This is CosineTable's twin, so there is some duplication.
 */
class SineTableTestSuite : public CxxTest::TestSuite {

	static const int kMinBitPrecision = 4;
	static const int kMaxBitPrecision = 16;

	// Precision is, as a precondition, expected to
	// range between 4 and 16.
	// We do not test above and below.

	public:
	void test_for_precision() {
		// See that precision is correctly held
		for (int p = kMinBitPrecision; p <= kMaxBitPrecision; p++) {
			Common::SineTable st(p);
			TS_ASSERT(st.getPrecision() == p);
		}
	}

	void test_common_angles() {
		// Test for common angles - at the moment, sin(0)==0
		for (int p = kMinBitPrecision; p <= kMaxBitPrecision; p++) {
			Common::SineTable st(p);
			float *table = const_cast<float*>(st.getTable());
			TS_ASSERT(table[0] == 0);
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
			// And now entries = 2^^(p-1)
			int half_entries = entries / 2;
			Common::SineTable st(p);
			float *table = const_cast<float*>(st.getTable());
			for (int i = 1; i < half_entries; i++) {
				TS_ASSERT(table[i] == table[entries - i]);
			}
		}
	}
};
