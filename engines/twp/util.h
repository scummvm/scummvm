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

#ifndef TWP_UTIL_H
#define TWP_UTIL_H

#include "common/formats/json.h"
#include "common/hashmap.h"
#include "common/rect.h"
#include "math/vector2d.h"
#include "math/matrix4.h"
#include "twp/ids.h"
#include "twp/objectanimation.h"

namespace Twp {
enum class Facing;
}

namespace Common {
template<>
struct Hash<Twp::Facing> : public UnaryFunction<Twp::Facing, uint> {
	uint operator()(Twp::Facing val) const { return (uint)val; }
};
} // namespace Common

namespace Twp {

class Object;

struct Vector2i {
	int x = 0;
	int y = 0;

	Vector2i() {}
	Vector2i(int x_, int y_) : x(x_), y(y_) {}
	Vector2i(float x_, float y_) : x(round(x_)), y(round(y_)) {}
	explicit Vector2i(const Math::Vector2d &p) : x(round(p.getX())), y(round(p.getY())) {}
	explicit operator Math::Vector2d() const {
		return Math::Vector2d(x, y);
	}

	Vector2i operator-(const Vector2i &v) const {
		return Vector2i(x - v.x, y - v.y);
	}

	Vector2i operator+(const Vector2i &v) const {
		return Vector2i(x + v.x, y + v.y);
	}

	Vector2i operator*(float f) const {
		return Vector2i(x * f, y * f);
	}

	Vector2i operator/(float f) const {
		return Vector2i(x / f, y / f);
	}
};

// general util
template<typename T, class DL = Common::DefaultDeleter<T> >
using unique_ptr = Common::ScopedPtr<T, DL>;

// game util
Facing getFacing(int dir, Facing facing);
Facing flip(Facing facing);
Facing getFacingToFaceTo(Common::SharedPtr<Object> actor, Common::SharedPtr<Object> obj);

// parsing util
bool toBool(const Common::JSONObject &jNode, const Common::String &key);
Math::Vector2d parseVec2(const Common::String &s);
Common::Rect parseRect(const Common::String &s);
void parseObjectAnimations(const Common::JSONArray &jAnims, Common::Array<ObjectAnimation> &anims);

// array util
template<typename T>
size_t find(const Common::Array<T> &array, const T &o) {
	for (size_t i = 0; i < array.size(); i++) {
		if (array[i] == o) {
			return i;
		}
	}
	return (size_t)-1;
}

template<typename T>
size_t find(const Common::Array<Common::SharedPtr<T> > &array, const T *o) {
	for (size_t i = 0; i < array.size(); i++) {
		if (array[i].get() == o) {
			return i;
		}
	}
	return (size_t)-1;
}

template<typename T>
size_t minIndex(const Common::Array<T> &values) {
	if (values.empty())
		return (size_t)-1;
	T min = values[0];
	size_t index = 0;
	for (size_t i = 1; i < values.size(); i++) {
		if (values[i] < min) {
			index = i;
			min = values[i];
		}
	}
	return index;
}

template<typename T>
Common::Array<T> reverse(const Common::Array<T> &arr) {
	Common::Array<T> result(arr.size());
	for (size_t i = 0; i < arr.size(); i++) {
		result[arr.size() - 1 - i] = arr[i];
	}
	return result;
}

// string util
Common::String join(const Common::Array<Common::String> &array, const Common::String &sep);
Common::String remove(const Common::String &txt, char startC, char endC);
Common::String replaceAll(const Common::String &s, const Common::String &what, const Common::String &by);

// math util
void scale(Math::Matrix4 &m, const Math::Vector2d &v);
Math::Vector2d operator*(const Math::Vector2d &v, float f);
float distance(const Math::Vector2d &p1, const Math::Vector2d &p2);
float distanceSquared(const Math::Vector2d &p1, const Math::Vector2d &p2);
float distanceToSegment(const Math::Vector2d &p, const Math::Vector2d &v, const Math::Vector2d &w);
float dot(const Math::Vector2d &u, const Math::Vector2d &v);
float length(const Math::Vector2d &v);
bool lineSegmentsCross(const Math::Vector2d &a, const Math::Vector2d &b, const Math::Vector2d &c, const Math::Vector2d &d);

} // namespace Twp

#endif
