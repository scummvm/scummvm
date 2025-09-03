#include <cxxtest/TestSuite.h>

#include "math/ray.h"

class RayTestSuite : public CxxTest::TestSuite {
public:
	// Test Constructors
	void test_Ray() {
		Math::Ray r;

		TS_ASSERT(r.getOrigin() == Math::Vector3d());
		TS_ASSERT(r.getDirection() == Math::Vector3d());

		Math::Vector3d o(3, 2, 1);
		Math::Vector3d d(0, 1, 2);

		Math::Ray r2(o, d);

		TS_ASSERT(r2.getOrigin() == o);
		TS_ASSERT(r2.getDirection() == d);
	}

	void test_translate() {
		Math::Vector3d o(3, 2, 1);
		Math::Vector3d d(0, 1, 2);
		Math::Ray r(o, d);

		r.translate(Math::Vector3d(0.5, 0.2, 0.1));
		TS_ASSERT(r.getDirection() == d);
		Math::Vector3d o2 = r.getOrigin();

		TS_ASSERT_DELTA(o2.x(), 3.5, 0.0001);
		TS_ASSERT_DELTA(o2.y(), 2.2, 0.0001);
		TS_ASSERT_DELTA(o2.z(), 1.1, 0.0001);
	}

	// TODO: Add tests for transform, rotate, rotateDirection, intersectAABB
	void test_intersectTriangle() {
		// A triangle that covers around the origin on the y plane.
		const Math::Vector3d v1(0, 0, -20);
		const Math::Vector3d v2(0, -10, 20);
		const Math::Vector3d v3(0, 10, 20);

		// A ray that points along the x axis, should hit the triangle at the origin
		Math::Ray r(Math::Vector3d(-9.5, 0, 0.7), Math::Vector3d(1, 0, 0));

		Math::Vector3d loc(7, 8, 9); // add values to ensure it's changed
		float dist = 99.0f;
		bool result = r.intersectTriangle(v1, v2, v3, loc, dist);
		// Should hit at the origin
		TS_ASSERT(result);
		TS_ASSERT_DELTA(dist, 9.5f, 0.0001);
		TS_ASSERT_DELTA(loc.x(), 0.0f, 0.0001);
		TS_ASSERT_DELTA(loc.y(), 0.0f, 0.0001);
		TS_ASSERT_DELTA(loc.z(), 0.7f, 0.0001);

		// A ray that points along the x axis in the opposite direction, should never hit the triangle
		Math::Ray r2(Math::Vector3d(-1, 0, 0), Math::Vector3d(-1, 0, 0));
		result = r2.intersectTriangle(v1, v2, v3, loc, dist);
		TS_ASSERT(!result);
	}
};
