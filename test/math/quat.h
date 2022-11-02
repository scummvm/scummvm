#include <cxxtest/TestSuite.h>

#include "math/quat.h"

class QuatTestSuite : public CxxTest::TestSuite {
public:
	// Test Constructors
	void test_Quaternion() {
		// Create an empty unit quaternion
		Math::Quaternion q;

		TS_ASSERT(q.x() == 0.0f);
		TS_ASSERT(q.y() == 0.0f);
		TS_ASSERT(q.z() == 0.0f);
		TS_ASSERT(q.w() == 1.0f);
	}

	// Test Normalization
	void test_Normalize() {
		Math::Quaternion q(1.0f, 2.0f, 3.0f, 4.0f);
		q.normalize();

		// Check the values against the known result
		TS_ASSERT(fabs(q.x() - 0.182574f) < 0.0001f);
		TS_ASSERT(fabs(q.y() - 0.365148f) < 0.0001f);
		TS_ASSERT(fabs(q.z() - 0.547723f) < 0.0001f);
		TS_ASSERT(fabs(q.w() - 0.730297f) < 0.0001f);

		// Check that the RMS is 1.0
		float r = sqrt(q.x() * q.x() + q.y() * q.y() + q.z() * q.z() + q.w() * q.w());
		TS_ASSERT(fabs(r - 1.0f) < 0.0001f);
	}

	// Test going to a matrix and back
	void test_quatMatrix() {
		Math::Angle xAngle(20);
		Math::Angle yAngle(30);
		Math::Angle zAngle(40);

		Math::Quaternion q = Math::Quaternion::fromEuler(xAngle, yAngle, zAngle, Math::EO_XYZ);
		Math::Matrix4 m = q.toMatrix();

		Math::Quaternion r(m);

		// Compare
		TS_ASSERT(fabs(q.x() - r.x()) < 0.0001f);
		TS_ASSERT(fabs(q.y() - r.y()) < 0.0001f);
		TS_ASSERT(fabs(q.z() - r.z()) < 0.0001f);
		TS_ASSERT(fabs(q.w() - r.w()) < 0.0001f);
	}

	// Test rotations
	void test_quatRotationXYZ() {
		Math::Angle xAngle(20);
		Math::Angle yAngle(30);
		Math::Angle zAngle(40);

		// First way
		Math::Quaternion q;
		q = Math::Quaternion::xAxis(xAngle);
		q *= Math::Quaternion::yAxis(yAngle);
		q *= Math::Quaternion::zAxis(zAngle);

		// Second way
		Math::Quaternion r = Math::Quaternion::fromEuler(xAngle, yAngle, zAngle, Math::EO_XYZ);

		// Compare to the known correct result
		TS_ASSERT(fabs(q.x() - 0.244792f) < 0.0001f);
		TS_ASSERT(fabs(q.y() - 0.182148f) < 0.0001f);
		TS_ASSERT(fabs(q.z() - 0.36758f) < 0.0001f);
		TS_ASSERT(fabs(q.w() - 0.878512f) < 0.0001f);

		// Compare to the known correct result
		TS_ASSERT(fabs(r.x() - 0.244792f) < 0.0001f);
		TS_ASSERT(fabs(r.y() - 0.182148f) < 0.0001f);
		TS_ASSERT(fabs(r.z() - 0.36758f) < 0.0001f);
		TS_ASSERT(fabs(r.w() - 0.878512f) < 0.0001f);
	}

	void test_quatAngleBetween() {
		Math::Angle a1(10);
		Math::Angle a2(20);
		Math::Quaternion q = Math::Quaternion::fromEuler(a1,0,0, Math::EO_XYZ);
		Math::Quaternion r = Math::Quaternion::fromEuler(a2,0,0, Math::EO_XYZ);

		Math::Angle a = q.getAngleBetween(r);

		TS_ASSERT(fabs(a.getDegrees() - 10) < 0.0001f);
	}

	void test_invert() {
		Math::Angle a1(50);
		Math::Angle a2(30);
		Math::Angle a3(10);

		Math::Quaternion q = Math::Quaternion::fromEuler(a1, a2, a3, Math::EO_XYZ);
		Math::Quaternion r = q.inverse();

		TS_ASSERT(r.x() == -q.x());
		TS_ASSERT(r.y() == -q.y());
		TS_ASSERT(r.z() == -q.z());
		TS_ASSERT(r.w() == q.w());
	}
};
