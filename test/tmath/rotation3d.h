#include <cxxtest/TestSuite.h>

#include "math/matrix4.h"

class Rotation3DTestSuite : public CxxTest::TestSuite {
public:
	void test_eulerXYX() {
		Math::Angle x(20);
		Math::Angle y(30);
		Math::Angle z(40);
		Math::Angle xx, yy, zz;

		// Test going to matrix and back
		Math::Matrix4 rotMat;
		rotMat.buildFromEuler(x, y, z, Math::EO_XYX);
		rotMat.getEuler(&xx, &yy, &zz, Math::EO_XYX);

		TS_ASSERT(x == xx);
		TS_ASSERT(y == yy);
		TS_ASSERT(z == zz);
	}

	void test_eulerXYZ() {
		Math::Angle x(20);
		Math::Angle y(30);
		Math::Angle z(40);
		Math::Angle xx, yy, zz;

		// Test going to matrix and back
		Math::Matrix4 rotMat;
		rotMat.buildFromEuler(x, y, z, Math::EO_XYZ);
		rotMat.getEuler(&xx, &yy, &zz, Math::EO_XYZ);

		TS_ASSERT(x == xx);
		TS_ASSERT(y == yy);
		TS_ASSERT(z == zz);
	}

	void test_eulerXZX() {
		Math::Angle x(20);
		Math::Angle y(30);
		Math::Angle z(40);
		Math::Angle xx, yy, zz;

		// Test going to matrix and back
		Math::Matrix4 rotMat;
		rotMat.buildFromEuler(x, y, z, Math::EO_XZX);
		rotMat.getEuler(&xx, &yy, &zz, Math::EO_XZX);

		TS_ASSERT(x == xx);
		TS_ASSERT(y == yy);
		TS_ASSERT(z == zz);
	}

	void test_eulerXZY() {
		Math::Angle x(20);
		Math::Angle y(30);
		Math::Angle z(40);
		Math::Angle xx, yy, zz;

		// Test going to matrix and back
		Math::Matrix4 rotMat;
		rotMat.buildFromEuler(x, y, z, Math::EO_XZY);
		rotMat.getEuler(&xx, &yy, &zz, Math::EO_XZY);

		TS_ASSERT(x == xx);
		TS_ASSERT(y == yy);
		TS_ASSERT(z == zz);
	}

	void test_eulerYXY() {
		Math::Angle x(20);
		Math::Angle y(30);
		Math::Angle z(40);
		Math::Angle xx, yy, zz;

		// Test going to matrix and back
		Math::Matrix4 rotMat;
		rotMat.buildFromEuler(x, y, z, Math::EO_YXY);
		rotMat.getEuler(&xx, &yy, &zz, Math::EO_YXY);

		TS_ASSERT(x == xx);
		TS_ASSERT(y == yy);
		TS_ASSERT(z == zz);
	}

	void test_eulerYXZ() {
		Math::Angle x(20);
		Math::Angle y(30);
		Math::Angle z(40);
		Math::Angle xx, yy, zz;

		// Test going to matrix and back
		Math::Matrix4 rotMat;
		rotMat.buildFromEuler(x, y, z, Math::EO_YXZ);
		rotMat.getEuler(&xx, &yy, &zz, Math::EO_YXZ);

		TS_ASSERT(x == xx);
		TS_ASSERT(y == yy);
		TS_ASSERT(z == zz);
	}

	void test_eulerYZX() {
		Math::Angle x(20);
		Math::Angle y(30);
		Math::Angle z(40);
		Math::Angle xx, yy, zz;

		// Test going to matrix and back
		Math::Matrix4 rotMat;
		rotMat.buildFromEuler(x, y, z, Math::EO_YZX);
		rotMat.getEuler(&xx, &yy, &zz, Math::EO_YZX);

		TS_ASSERT(x == xx);
		TS_ASSERT(y == yy);
		TS_ASSERT(z == zz);
	}

	void test_eulerYZY() {
		Math::Angle x(20);
		Math::Angle y(30);
		Math::Angle z(40);
		Math::Angle xx, yy ,zz;

		// Test going to matrix and back
		Math::Matrix4 rotMat;
		rotMat.buildFromEuler(x, y, z, Math::EO_YZY);
		rotMat.getEuler(&xx, &yy, &zz, Math::EO_YZY);

		TS_ASSERT(x == xx);
		TS_ASSERT(y == yy);
		TS_ASSERT(z == zz);
	}

	void test_eulerZXY() {
		Math::Angle x(20);
		Math::Angle y(30);
		Math::Angle z(40);
		Math::Angle xx, yy, zz;

		// Test going to matrix and back
		Math::Matrix4 rotMat;
		rotMat.buildFromEuler(x, y, z, Math::EO_ZXY);
		rotMat.getEuler(&xx, &yy, &zz, Math::EO_ZXY);

		TS_ASSERT(x == xx);
		TS_ASSERT(y == yy);
		TS_ASSERT(z == zz);
	}

	void test_eulerZXZ() {
		Math::Angle x(20);
		Math::Angle y(30);
		Math::Angle z(40);
		Math::Angle xx, yy, zz;

		// Test going to matrix and back
		Math::Matrix4 rotMat;
		rotMat.buildFromEuler(x, y, z, Math::EO_ZXZ);
		rotMat.getEuler(&xx, &yy, &zz, Math::EO_ZXZ);

		TS_ASSERT(x == xx);
		TS_ASSERT(y == yy);
		TS_ASSERT(z == zz);
	}

	void test_eulerZYX() {
		Math::Angle x(20);
		Math::Angle y(30);
		Math::Angle z(40);
		Math::Angle xx, yy, zz;

		// Test going to matrix and back
		Math::Matrix4 rotMat;
		rotMat.buildFromEuler(x, y, z, Math::EO_ZYX);
		rotMat.getEuler(&xx, &yy, &zz, Math::EO_ZYX);

		TS_ASSERT(x == xx);
		TS_ASSERT(y == yy);
		TS_ASSERT(z == zz);
	}

	void test_eulerZYZ() {
		Math::Angle x(20);
		Math::Angle y(30);
		Math::Angle z(40);
		Math::Angle xx, yy, zz;

		// Test going to matrix and back
		Math::Matrix4 rotMat;
		rotMat.buildFromEuler(x, y, z, Math::EO_ZYZ);
		rotMat.getEuler(&xx, &yy, &zz, Math::EO_ZYZ);

		TS_ASSERT(x == xx);
		TS_ASSERT(y == yy);
		TS_ASSERT(z == zz);
	}
};
