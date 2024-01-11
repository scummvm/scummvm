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

#include "twp/ids.h"
#include "twp/objectanimation.h"
#include "math/vector2d.h"
#include "math/matrix4.h"
#include "common/formats/json.h"
#include "common/rect.h"

namespace Twp {

class Object;

// general util
template<typename T, class DL = Common::DefaultDeleter<T> >
using unique_ptr = Common::ScopedPtr<T, DL>;

template<typename T>
T clamp(T x, T a, T b) {
	if (x < a)
		return a;
	if (x > b)
		return b;
	return x;
}

// game util
Facing getFacing(int dir, Facing facing);
Facing flip(Facing facing);
Facing getFacingToFaceTo(Object *actor, Object *obj);

// parsing util
bool toBool(const Common::JSONObject &jNode, const Common::String &key);
Math::Vector2d parseVec2(const Common::String &s);
Common::Rect parseRect(const Common::String &s);
void parseObjectAnimations(const Common::JSONArray &jAnims, Common::Array<ObjectAnimation> &anims);

// array util
template<typename T>
int find(const Common::Array<T>& array, const T& o) {
	for (int i = 0; i < array.size(); i++) {
		if (array[i] == o) {
			return i;
		}
	}
	return -1;
}

// string util
Common::String join(const Common::Array<Common::String>& array, const Common::String& sep);
Common::String replace(const Common::String& s, const Common::String& what, const Common::String& by);
Common::String remove(const Common::String &txt, char startC, char endC);

// math util
void scale(Math::Matrix4 &m, const Math::Vector2d &v);
Math::Vector2d operator*(Math::Vector2d v, float f);
float distance(Math::Vector2d p1, Math::Vector2d p2);
float distanceSquared(Math::Vector2d p1, Math::Vector2d p2);
float distanceToSegment(Math::Vector2d p, Math::Vector2d v, Math::Vector2d w);

} // namespace Twp

#endif
