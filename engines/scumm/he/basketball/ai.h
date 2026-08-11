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

#ifndef SCUMM_HE_BASKETBALL_AI_H
#define SCUMM_HE_BASKETBALL_AI_H

#ifdef ENABLE_HE

#include "scumm/he/intern_he.h"
#include "scumm/he/logic_he.h"

#include "scumm/he/basketball/collision/bball_collision_support_obj.h"


namespace Scumm {

#define FLOAT_EPSILON  0.001F

inline int square(int x) { return x * x; }

class Line2D {
public:
	Line2D(float x_coef, float y_coef, float constant) {
		_a = x_coef;
		_b = y_coef;
		_c = constant;
	}

	Line2D(const U32FltVector2D &point1, const U32FltVector2D &point2) {
		LineFromTwoPoints(point1, point2);
	}

	void LineFromTwoPoints(U32FltVector2D pt1, U32FltVector2D pt2) {
		float temp = (pt2.x - pt1.x);

		if (fabs(temp) < FLOAT_EPSILON) {
			assert(((fabs(pt2.y - pt1.y) >= FLOAT_EPSILON)));
			_a = 1;
			_b = 0;
		} else {
			float m = (pt2.y - pt1.y) / temp;
			_a = -m;
			_b = 1;
		}

		_c = -(_a * pt2.x + _b * pt2.y);
	}

	inline float distance(U32FltVector2D point) {
		return fabs((_a * point.x + _b * point.y + _c) / sqrt(square(_a) + square(_b)));
	}

	inline float distance2(U32FltVector2D point) {
		return fabs(square(_a * point.x + _b * point.y + _c) / (square(_a) + square(_b)));
	}

	inline float angle() {
		return atan2(-_a, _b);
	}

	bool inBetween(U32FltVector2D point, U32FltVector2D end1, U32FltVector2D end2) {
		assert((!onLine(end1) || !onLine(end2)));

		point = projectPoint(point);
		float distance2 = end1.distance2(end2);

		return (point.distance2(end1) <= distance2 && point.distance2(end2) <= distance2) ? true : false;
	}

	bool onLine(U32FltVector2D point) {
		return (distance2(point) < 1.0F) ? true : false;
	}

	U32FltVector2D projectPoint(U32FltVector2D point) {
		return intersection(perpendicular(point));
	}

	float getY(float x) {
		if (_b != 0.0F)
			return (-_a * x - _c) / _b;

		return 0.0F;
	}

	float getX(float y) {
		if (_a != 0.0F)
			return (-_b * y - _c) / _a;

		return 0.0F;
	}

	U32FltVector2D intersection(Line2D line) {
		U32FltVector2D result(0.0F, 0.0F);

		assert(!sameSlope(line));

		if (_b == 0.0F) {
			result.x = -_c / _a;
			result.y = line.getY(result.x);
			return result;
		}

		if (line._b == 0.0F) {
			result.x = -line._c / line._a;
			result.y = getY(result.y);
			return result;
		}

		result.x = (_c * line._b - _b * line._c) / (line._a * _b - _a * line._b);
		result.y = getY(result.x);
		return result;
	}

	Line2D perpendicular(U32FltVector2D point) {
		return Line2D(_b, -_a, _a * point.y - _b * point.x);
	}

	Line2D shiftY(float val) {
		return Line2D(_a, _b, _c - val * _b);
	}

	Line2D shiftX(float val) {
		return Line2D(_a, _b, _c - val * _a);
	}

	// Returns whether the projection of point1 is closer to targPoint than the projection of point2
	bool isPointCloserToPointOnLine(U32FltVector2D point1, U32FltVector2D point2, U32FltVector2D targPoint) {

		assert(!onLine(targPoint));

		point1 = projectPoint(point1);
		point2 = projectPoint(point2);

		return (point1.distance(targPoint) < point2.distance(targPoint)) ? true : false;
	}

	bool halfPlaneTest(U32FltVector2D point) {
		if (_b == 0)
			return (point.x < -_c / _a) ? true : false;

		return (point.y > getY(point.x)) ? true : false;
	}

	bool sameSlope(Line2D line) {
		return ((_b == 0 && line._b == 0) || ((_a / _b) == (line._a / line._b))) ? true : false;
	}

private:
	float _a, _b, _c; // The three coeffs in the line equation:
					  // Ax + By + C = 0
};

} // End of namespace Scumm

#endif // ENABLE_HE

#endif // SCUMM_HE_BASKETBALL_AI_H
