/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SCUMM_HE_BASKETBALL_COLLISION_BBALL_COLLISION_SUPPORT_OBJ_H
#define SCUMM_HE_BASKETBALL_COLLISION_BBALL_COLLISION_SUPPORT_OBJ_H

#ifdef ENABLE_HE

#include "common/std/algorithm.h"

namespace Scumm {

#define BBALL_M_PI         3.14159265358979
#define COPY_SIGN(x, y)    ((x < 0) == (y < 0)) ? x : -x

#define MIN_GREATER_THAN_ZERO(a, b) ((a < 0) ? ((b < 0) ? ((a > b) ? a : b) : b) : ((b < 0) ? a : ((a > b) ? b : a)))

enum EDimension {
	X_INDEX = 0,
	Y_INDEX = 1,
	Z_INDEX = 2
};

enum ERevDirection {
	kClockwise = -1,
	kNone = 0,
	kCounterClockwise = 1
};

template<class Type>
class U32Construct2D {

public:
	Type x;
	Type y;

	U32Construct2D() : x(0), y(0) {}
	U32Construct2D(Type xIn, Type yIn) : x(xIn), y(yIn) {}

	bool operator==(const U32Construct2D<Type> &other) const {
		return ((other.x == x) && (other.y == y));
	}

	U32Construct2D<Type> operator-(const U32Construct2D<Type> &other) const {
		U32Construct2D<Type> newConstruct;

		newConstruct.x = x - other.x;
		newConstruct.y = y - other.y;

		return newConstruct;
	}

	U32Construct2D<Type> operator+(const U32Construct2D<Type> &other) const {
		U32Construct2D<Type> newConstruct;

		newConstruct.x = x + other.x;
		newConstruct.y = y + other.y;

		return newConstruct;
	}

	Type operator*(const U32Construct2D<Type> &multiplier) const {
		return (x * multiplier.x + y * multiplier.y);
	}

	Type &operator[](EDimension dimension) {
		assert(dimension <= Y_INDEX);
		return *(&x + dimension);
	}

	const Type &operator[](EDimension dimension) const {
		assert(dimension <= Y_INDEX);
		return *(&x + dimension);
	}

	Type magnitude(void) const {
		return (Type)(sqrt(x * x + y * y));
	}
};

template<class Type>
class U32Construct3D {

public:
	Type x;
	Type y;
	Type z;

	U32Construct3D() : x(0), y(0), z(0) {}
	U32Construct3D(Type xIn, Type yIn, Type zIn) : x(xIn), y(yIn), z(zIn) {}

	Type magnitude(void) const {
		return (Type)(sqrt(x * x + y * y + z * z));
	}

	Type xyMagnitude(void) const {
		return (Type)(sqrt(x * x + y * y));
	}

	bool operator==(const U32Construct3D<Type> &other) const {
		return ((other.x == x) && (other.y == y) && (other.z == z));
	}

	bool operator!=(const U32Construct2D<Type> &other) const {
		return ((other.x != x) || (other.y != y));
	}

	Type &operator[](EDimension dimension) {
		assert(dimension <= Z_INDEX);
		return *(&x + dimension);
	}

	const Type &operator[](EDimension dimension) const {
		assert(dimension <= Z_INDEX);
		return *(&x + dimension);
	}

	U32Construct3D<Type> operator+(const U32Construct3D<Type> &other) const {
		U32Construct3D<Type> newConstruct;

		newConstruct.x = x + other.x;
		newConstruct.y = y + other.y;
		newConstruct.z = z + other.z;

		return newConstruct;
	}

	Type operator*(const U32Construct3D<Type> &multiplier) const {
		return (x * multiplier.x + y * multiplier.y + z * multiplier.z);
	}

	friend U32Construct3D<Type> operator*(const Type multiplier1[4][4], U32Construct3D<Type> multiplier2) {
		U32Construct3D<Type> newPoint;
		Type h = 0;
		int column, row;

		for (row = X_INDEX; row <= Z_INDEX; ++row) {
			for (column = X_INDEX; column <= Z_INDEX; ++column) {
				newPoint[(EDimension)row] += multiplier1[row][column] * multiplier2[(EDimension)column];
			}

			newPoint[(EDimension)row] += multiplier1[row][column];
		}

		for (column = X_INDEX; column <= Z_INDEX; ++column) {
			h += multiplier1[row][column] * multiplier2[(EDimension)column];
		}

		h += multiplier1[row][column];

		(h == 0) ? 0 : newPoint.x /= h;
		(h == 0) ? 0 : newPoint.y /= h;
		(h == 0) ? 0 : newPoint.z /= h;

		return newPoint;
	}
};

template<class Type>
class U32Point3D;

template<class Type>
class U32Point2D : public U32Construct2D<Type> {
public:
	U32Point2D() : U32Construct2D<Type>() {}
	U32Point2D(Type xx, Type yy) : U32Construct2D<Type>(xx, yy) {}
	U32Point2D(const U32Construct2D<Type> &other) : U32Construct2D<Type>(other.x, other.y) {}
	U32Point2D(const U32Construct3D<Type> &other) : U32Construct2D<Type>(other.x, other.y) {} // For 3D

	U32Point2D<Type> operator=(const U32Construct3D<Type> &other) {
		this->x = other.x;
		this->y = other.y;
		return *this;
	}

	U32Construct2D<Type> operator-(const U32Construct3D<Type> &other) const {
		U32Construct2D<Type> newPoint;

		newPoint.x = this->x - other.x;
		newPoint.y = this->y - other.y;

		return newPoint;
	}

	U32Construct2D<Type> operator-(const U32Construct2D<Type> &other) const {
		U32Construct2D<Type> newPoint;

		newPoint.x = this->x - other.x;
		newPoint.y = this->y - other.y;

		return newPoint;
	}
};

typedef U32Point2D<int> U32IntPoint2D;
typedef U32Point2D<float> U32FltPoint2D;

template<class Type>
class U32Point3D : public U32Construct3D<Type> {
public:
	U32Point3D() : U32Construct3D<Type>() {}
	U32Point3D(Type xIn, Type yIn, Type zIn) : U32Construct3D<Type>(xIn, yIn, zIn) {}
	U32Point3D(const U32Construct3D<Type> &other) : U32Construct3D<Type>(other.x, other.y, other.z) {}

	bool operator==(const U32Construct2D<Type> &other) const {
		return ((other.x == this->x) && (other.y == this->y));
	}

	bool operator==(const U32Construct3D<Type> &other) const {
		return ((other.x == this->x) && (other.y == this->y) && (other.z == this->z));
	}

	U32Construct3D<Type> operator-(const U32Construct2D<Type> &other) const {
		U32Construct3D<Type> newPoint;

		newPoint.x = this->x - other.x;
		newPoint.y = this->y - other.y;
		newPoint.z = this->z;

		return newPoint;
	}

	U32Construct3D<Type> operator-(const U32Construct3D<Type> &other) const {
		U32Construct3D<Type> newPoint;

		newPoint.x = this->x - other.x;
		newPoint.y = this->y - other.y;
		newPoint.z = this->z - other.z;

		return newPoint;
	}
};

typedef U32Point3D<int> U32IntPoint3D;
typedef U32Point3D<float> U32FltPoint3D;

template<class Type>
class U32Vector2D : public U32Construct2D<Type> {
public:
	U32Vector2D() : U32Construct2D<Type>() {}
	U32Vector2D(Type xx, Type yy) : U32Construct2D<Type>(xx, yy) {}
	U32Vector2D(const U32Construct2D<Type> &other) : U32Construct2D<Type>(other.x, other.y) {}
	U32Vector2D(const U32Construct3D<Type> &other) : U32Construct2D<Type>(other.x, other.y) {}

	U32Vector2D<Type> normalize() const {
		Type magnitude = this->magnitude();
		assert(magnitude > 0);

		U32Vector2D<Type> newVector;
		newVector.x = this->x / magnitude;
		newVector.y = this->y / magnitude;

		return newVector;
	}

	U32Vector2D<Type> operator*(Type multiplier) const {
		U32Vector2D<Type> newVector;

		newVector.x = multiplier * this->x;
		newVector.y = multiplier * this->y;

		return newVector;
	}

	Type operator*(const U32Vector2D<Type> &multiplier) const {
		return (this->x * multiplier.x + this->y * multiplier.y);
	}

	const U32Vector2D<Type> &operator*=(Type multiplier) {
		*this = *this * multiplier;
		return *this;
	}

	const U32Vector2D<Type> &rotate(ERevDirection whichDirection, double radians) {
		U32Point2D<Type> newPoint;

		if (whichDirection == kCounterClockwise) {
			newPoint.x = this->x * cos(radians) - this->y * sin(radians);
			newPoint.y = this->x * sin(radians) + this->y * cos(radians);
		} else {
			newPoint.x = this->x * cos(radians) + this->y * sin(radians);
			newPoint.y = this->y * cos(radians) - this->x * sin(radians);
		}

		this->x = newPoint.x;
		this->y = newPoint.y;
		return *this;
	}

	U32Construct3D<Type> cross(const U32Vector2D<Type> &otherVector) const {
		U32Construct3D<Type> newVector;

		newVector.x = 0;
		newVector.y = 0;
		newVector.z = this->x * otherVector.y - this->y * otherVector.x;

		return newVector;
	}

	U32Vector2D<Type> transpose(void) const {
		U32Vector2D<Type> newVector;
		newVector.x = -this->y;
		newVector.y = this->x;

		return newVector;
	}

	Type distance2(const U32Vector2D<Type> &otherVector) const {
		return ((this->x - otherVector.x) * (this->x - otherVector.x) +
				(this->y - otherVector.y) * (this->y - otherVector.y));
	}

	Type distance(const U32Vector2D<Type> &otherVector) const {
		return (Type)(sqrt((double)(distance2(otherVector))));
	}

	ERevDirection getRevDirection(const U32Vector2D<Type> &otherVector) const {
		U32Construct3D<Type> vector3 = this->cross(otherVector);

		if (vector3.z > 0) {
			return kCounterClockwise;
		} else if (vector3.z < 0) {
			return kClockwise;
		} else {
			return kNone;
		}
	}

	// Project this vector onto otherVector, and return the resulting vector's magnitude.
	Type projectScalar(const U32Vector2D<Type> &otherVector) const {
		Type projectionScalar;

		if (otherVector.magnitude() == 0) {
			projectionScalar = 0;
		} else {
			float otherMagnitude = otherVector.magnitude();
			projectionScalar = (*this * otherVector) / otherMagnitude;
		}

		return projectionScalar;
	}
};

typedef U32Vector2D<int> U32IntVector2D;
typedef U32Vector2D<float> U32FltVector2D;

template<class Type>
class U32Vector3D : public U32Construct3D<Type> {
public:
	U32Vector3D() : U32Construct3D<Type>() {}
	U32Vector3D(Type xx, Type yy, Type zz) : U32Construct3D<Type>(xx, yy, zz) {}
	U32Vector3D<Type>(const U32Construct2D<Type> &other) : U32Construct3D<Type>(other.x, other.y, 0) {}
	U32Vector3D<Type>(const U32Construct3D<Type> &other) : U32Construct3D<Type>(other.x, other.y, other.z) {}

	U32Vector3D<Type> operator-(const U32Construct2D<Type> &other) const {
		U32Vector3D<Type> newVector;

		newVector.x = this->x - other.x;
		newVector.y = this->y - other.y;
		newVector.z = this->z;

		return newVector;
	}

	U32Vector3D<Type> operator-(const U32Construct3D<Type> &other) const {
		U32Vector3D<Type> newVector;

		newVector.x = this->x - other.x;
		newVector.y = this->y - other.y;
		newVector.z = this->z - other.z;

		return newVector;
	}

	Type operator*(const U32Construct3D<Type> &multiplier) const {
		return (this->x * multiplier.x + this->y * multiplier.y + this->z * multiplier.z);
	}

	U32Vector3D<Type> &operator+=(const U32Construct3D<Type> &adder) const {
		*this = *this + adder;
		return *this;
	}

	U32Vector3D<Type> &operator-=(const U32Construct3D<Type> &subtractor) {
		*this = *this - subtractor;
		return *this;
	}

	U32Vector3D<Type> operator*(Type multiplier) const {
		U32Vector3D<Type> newVector;

		newVector.x = multiplier * this->x;
		newVector.y = multiplier * this->y;
		newVector.z = multiplier * this->z;

		return newVector;
	}

	const U32Vector3D<Type> &operator*=(Type multiplier) {
		*this = *this * multiplier;
		return *this;
	}

	U32Vector3D<Type> operator/(Type divider) const {
		U32Vector3D<Type> newVector;

		newVector.x = (divider == 0) ? 0 : this->x / divider;
		newVector.y = (divider == 0) ? 0 : this->y / divider;
		newVector.z = (divider == 0) ? 0 : this->z / divider;

		return newVector;
	}

	const U32Vector3D<Type> &operator/=(Type multiplier) {
		*this = *this / multiplier;
		return *this;
	}

	U32Vector3D<Type> cross(const U32Vector3D<Type> &otherVector) const {
		U32Vector3D<Type> newVector;

		newVector.x = this->y * otherVector.z - this->z * otherVector.y;
		newVector.y = this->z * otherVector.x - this->x * otherVector.z;
		newVector.z = this->x * otherVector.y - this->y * otherVector.x;

		return newVector;
	}

	bool operator>(const U32Vector3D<Type> &otherVector) const {
		return (this->magnitude() > otherVector.magnitude());
	}

	bool operator<(const U32Vector3D<Type> &otherVector) const {
		return (this->magnitude() < otherVector.magnitude());
	}

	// Project this vector onto otherVector, and return the resulting vector
	U32Vector3D<Type> projectVector(const U32Vector3D<Type> &otherVector) const {
		Type projectionScalar;

		if (otherVector.magnitude() == 0)
			projectionScalar = 0;
		else
			projectionScalar = (*this * otherVector) / (otherVector.magnitude() * otherVector.magnitude());

		U32Vector3D<Type> newVector = otherVector * projectionScalar;
		return newVector;
	}

	// Project this vector onto otherVector, and return the resulting vector's magnitude
	Type projectScalar(const U32Vector3D<Type> &otherVector) const {
		Type projectionScalar;

		if (otherVector.magnitude() == 0) {
			projectionScalar = 0;
		} else {
			float otherMagnitude = otherVector.magnitude();
			projectionScalar = (*this * otherVector) / otherMagnitude;
		}

		return projectionScalar;
	}

	U32Vector3D<Type> normalize() const {
		Type magnitude = this->magnitude();
		assert(magnitude > 0);

		U32Vector3D<Type> newVector;

		if (magnitude != 0) {
			newVector.x = this->x / magnitude;
			newVector.y = this->y / magnitude;
			newVector.z = this->z / magnitude;
		}

		return newVector;
	}
};

typedef U32Vector3D<int> U32IntVector3D;
typedef U32Vector3D<float> U32FltVector3D;

class U32Distance3D : public U32Construct3D<float> {
public:
	U32Distance3D() : U32Construct3D<float>() {}
	U32Distance3D(const U32Construct2D<float> &other) : U32Construct3D<float>(other.x, other.y, 0) {}
	U32Distance3D(const U32Construct3D<float> &other) : U32Construct3D<float>(other.x, other.y, other.z) {}

	U32Distance3D operator-(int subtractor) const {
		U32Distance3D newDistance;

		if (magnitude() != 0) {
			newDistance.x = x - ((x * subtractor) / magnitude());
			newDistance.y = y - ((y * subtractor) / magnitude());
			newDistance.z = z - ((z * subtractor) / magnitude());
		}

		return newDistance;
	}

	U32Distance3D operator-=(int subtractor) {
		*this = *this - subtractor;
		return *this;
	}
};

struct U32Circle {

	U32FltPoint2D center;
	float radius;
};

struct U32Sphere {

	U32FltPoint3D center;
	float radius;
};

struct U32Cylinder : public U32Sphere {
	float height;
};

struct U32BoundingBox {

	U32IntPoint3D minPoint;
	U32IntPoint3D maxPoint;

	void setMinPoint(int x, int y) {
		minPoint.x = x;
		minPoint.y = y;
	}

	void setMaxPoint(int x, int y) {
		maxPoint.x = x;
		maxPoint.y = y;
	}

	const U32IntPoint3D &getMinPoint(void) const {
		return minPoint;
	}

	const U32IntPoint3D &getMaxPoint(void) const {
		return maxPoint;
	}

	bool intersect(const U32BoundingBox &targetBox) const {
		return ((((minPoint.x <= targetBox.minPoint.x) &&
				  (maxPoint.x >= targetBox.minPoint.x)) ||

				 ((minPoint.x <= targetBox.maxPoint.x) &&
				  (maxPoint.x >= targetBox.maxPoint.x)) ||

				 ((targetBox.minPoint.x <= minPoint.x) &&
				  (targetBox.maxPoint.x >= minPoint.x)) ||

				 ((targetBox.minPoint.x <= maxPoint.x) &&
				  (targetBox.maxPoint.x >= maxPoint.x)))

				&&

				(((minPoint.y <= targetBox.minPoint.y) &&
				  (maxPoint.y >= targetBox.minPoint.y)) ||

				 ((minPoint.y <= targetBox.maxPoint.y) &&
				  (maxPoint.y >= targetBox.maxPoint.y)) ||

				 ((targetBox.minPoint.y <= minPoint.y) &&
				  (targetBox.maxPoint.y >= minPoint.y)) ||

				 ((targetBox.minPoint.y <= maxPoint.y) &&
				  (targetBox.maxPoint.y >= maxPoint.y))));
	}

	const U32IntPoint3D &operator[](int point) const {
		assert(point <= 1);
		return ((point == 0) ? minPoint : maxPoint);
	}

	U32IntPoint3D &operator[](int point) {
		assert(point <= 1);
		return ((point == 0) ? minPoint : maxPoint);
	}

	bool isPointWithin(U32FltPoint2D point) const {
		return ((minPoint.x <= point.x) && (point.x <= maxPoint.x) &&
				(minPoint.y <= point.y) && (point.y <= maxPoint.y));
	}
};

struct U32Plane {
	U32FltPoint3D point;
	U32FltVector3D normal;
	float collisionEfficiency;
	float friction;

	// Average this plane with another plane
	U32Plane &average(const U32Plane &otherPlane) {
		point.x = (point.x + otherPlane.point.x) / 2;
		point.y = (point.y + otherPlane.point.y) / 2;
		point.z = (point.z + otherPlane.point.z) / 2;

		normal.x = (normal.x + otherPlane.normal.x) / 2;
		normal.y = (normal.y + otherPlane.normal.y) / 2;
		normal.z = (normal.z + otherPlane.normal.z) / 2;

		float normalMag = normal.magnitude();
		normal.x = (normalMag == 0) ? 0 : normal.x / normalMag;
		normal.y = (normalMag == 0) ? 0 : normal.y / normalMag;
		normal.z = (normalMag == 0) ? 0 : normal.z / normalMag;

		collisionEfficiency = (collisionEfficiency + otherPlane.collisionEfficiency) / 2;
		friction = (friction + otherPlane.friction) / 2;

		return *this;
	}

	void clear() {
		point = U32FltPoint3D();
		normal = U32FltVector3D();
		collisionEfficiency = 0.0F;
		friction = 0.0F;
	}
};

class U32Ray2D {
public:
	U32FltPoint2D _origin;
	U32FltVector2D _direction;

	bool intersection(const U32Ray2D &otherRay, U32FltPoint2D *intersection) const {
		float numerator = ((otherRay._origin - _origin) * otherRay._direction.transpose());
		float denominator = _direction * otherRay._direction.transpose();

		if ((denominator == 0) ||
			(denominator < 0) != (numerator < 0)) {
			return false;
		} else {
			float s = numerator / denominator;
			assert(s >= 0);

			intersection->x = _origin.x + (_direction.x * s);
			intersection->y = _origin.y + (_direction.y * s);
			return true;
		}
	}

	bool nearIntersection(const U32Circle &circle, U32FltPoint2D *intersection) const {
		U32FltVector2D coVector = _origin - circle.center;
		double b = _direction.normalize() * coVector;
		double c = (coVector * coVector) - (circle.radius * circle.radius);
		double d = (b * b) - c;

		if (d < 0) {
			return false;
		} else {
			double t = -b - sqrt(d);

			if (t < 0) {
				return false;
			} else {
				*intersection = _origin + (_direction.normalize() * t);
				return true;
			}
		}
	}

	bool farIntersection(const U32Circle &circle, U32FltPoint2D *intersection) const {
		U32FltVector2D coVector = _origin - circle.center;
		double b = _direction.normalize() * coVector;
		double c = (coVector * coVector) - (circle.radius * circle.radius);
		double d = (b * b) - c;

		if (d < 0) {
			return false;
		} else {
			double t = -b + sqrt(d);

			if (t < 0) {
				return false;
			} else {
				*intersection = _origin + (_direction.normalize() * t);
				return true;
			}
		}
	}
};

class U32Ray3D {
public:
	U32FltPoint3D _origin;
	U32FltVector3D _direction;
};

} // End of namespace Scumm

#endif // ENABLE_HE

#endif // SCUMM_HE_BASKETBALL_COLLISION_BBALL_COLLISION_SUPPORT_OBJ_H
