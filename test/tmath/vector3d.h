#include <cxxtest/TestSuite.h>

#include "math/vector3d.h"

class Vector3dTestSuite : public CxxTest::TestSuite {
public:
	// Test Constructors
	void test_Vector3d() {
		Math::Vector3d v;

		TS_ASSERT(v.x() == 0.0f);
		TS_ASSERT(v.y() == 0.0f);
		TS_ASSERT(v.z() == 0.0f);

		Math::Vector3d v2(3, 2.2, 1);

		TS_ASSERT(v2.x() == 3.0f);
		TS_ASSERT(v2.y() == 2.2f);
		TS_ASSERT(v2.z() == 1.0f);

		Math::Vector3d v3(v2);

		TS_ASSERT(v3.x() == 3.0f);
		TS_ASSERT(v3.y() == 2.2f);
		TS_ASSERT(v3.z() == 1.0f);
	}

	void test_set() {
		Math::Vector3d v(2, 4, 6);
		v.set(1, 2, 3);

		TS_ASSERT(v.x() == 1.0f);
		TS_ASSERT(v.y() == 2.0f);
		TS_ASSERT(v.z() == 3.0f);
	}

	void test_crossProduct() {
		Math::Vector3d v1(1, 0, 0);
		Math::Vector3d v2(0, 1, 0);

		Math::Vector3d c12 = Math::Vector3d::crossProduct(v1, v2);
		Math::Vector3d c21 = Math::Vector3d::crossProduct(v2, v1);

		TS_ASSERT(c12 == Math::Vector3d(0, 0, 1));
		TS_ASSERT(c21 == Math::Vector3d(0, 0, -1));
	}

	void test_length() {
		Math::Vector3d v(1, 0, 1);
		TS_ASSERT_DELTA(v.length(), sqrt(2), 0.0001);

		Math::Vector3d v2(2, 2, 2);
		TS_ASSERT_DELTA(v2.length(), sqrt(12), 0.0001);

		// check static version too
		TS_ASSERT_DELTA(Math::Vector3d::length(v2), v2.length(), 0.0001);
	}

	void test_interpolate() {
		Math::Vector3d v1(1, 0, 2);
		Math::Vector3d v2(0, 5, 3);

		Math::Vector3d int1 = Math::Vector3d::interpolate(v1, v2, 0.1);
		TS_ASSERT_DELTA(int1.x(), 0.9, 0.0001);
		TS_ASSERT_DELTA(int1.y(), 0.5, 0.0001);
		TS_ASSERT_DELTA(int1.z(), 2.1, 0.0001);
	}

	/* all the below tests are for functions in Math::Vector, but add tests
	 * here as we can't directly instantiate that */

	void test_setValue() {
		Math::Vector3d v(3, 7, 9);

		v.setValue(0, 1);
		TS_ASSERT(v.x() == 1.0f);
		TS_ASSERT(v.y() == 7.0f);
		TS_ASSERT(v.z() == 9.0f);

		v.setValue(1, 3);
		v.setValue(2, 2);
		TS_ASSERT(v.x() == 1.0f);
		TS_ASSERT(v.y() == 3.0f);
		TS_ASSERT(v.z() == 2.0f);
	}

	void test_getValue() {
		Math::Vector3d v(5, 6, 7);

		TS_ASSERT(v.getValue(0) == 5.0f);
		TS_ASSERT(v.getValue(1) == 6.0f);
		TS_ASSERT(v.getValue(2) == 7.0f);
	}

	void test_dotProduct() {
		Math::Vector3d v1(1, 2, 3);
		Math::Vector3d v2(6, 5, 4);

		float result = v2.dotProduct(v1);

		TS_ASSERT_DELTA(result, 28.0f, 0.0001f);
	}

	void test_dotProductOrthogonal() {
		Math::Vector3d v1(6, 0, 0);
		Math::Vector3d v2(0, 9, 0);

		float result = v2.dotProduct(v1);

		TS_ASSERT_EQUALS(result, 0.0f);
	}

	void test_normalizeTrivial() {
		Math::Vector3d v(0, 1, 0);

		Math::Vector3d vn = v.getNormalized();

		v.normalize();

		TS_ASSERT_EQUALS(vn, v);
		TS_ASSERT(v.x() == 0);
		TS_ASSERT(v.y() == 1);
		TS_ASSERT(v.z() == 0);
	}

	void test_normalize() {
		Math::Vector3d v(2, 4, 6);

		Math::Vector3d vn = v.getNormalized();
		v.normalize();

		TS_ASSERT_EQUALS(vn, v);
		TS_ASSERT_DELTA(v.x(), 2 / sqrt(56), 0.0001);
		TS_ASSERT_DELTA(v.y(), 4 / sqrt(56), 0.0001);
		TS_ASSERT_DELTA(v.z(), 6 / sqrt(56), 0.0001);
	}

	void test_magnitude() {
		Math::Vector3d v(3, 2.5, 1);

		TS_ASSERT_DELTA(v.getMagnitude(), sqrt(16.25), 0.0001);
	}

};
