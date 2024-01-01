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
#include "twp/object.h"
#include "math/vector2d.h"
#include "common/formats/json.h"

namespace Twp {

template<typename T>
T clamp(T x, T a, T b) {
	if (x < a)
		return a;
	if (x > b)
		return b;
	return x;
}

Math::Vector2d operator*(Math::Vector2d v, float f);

Facing getFacing(int dir, Facing facing);
Facing flip(Facing facing);
Facing getFacingToFaceTo(Object *actor, Object *obj);

bool toBool(const Common::JSONObject &jNode, const Common::String &key);
Math::Vector2d parseVec2(const Common::String &s);
Common::Rect parseRect(const Common::String &s);
void parseObjectAnimations(const Common::JSONArray &jAnims, Common::Array<ObjectAnimation> &anims);

float distance(Math::Vector2d p1, Math::Vector2d p2);
float distanceSquared(Math::Vector2d p1, Math::Vector2d p2);
float distanceToSegment(Math::Vector2d p, Math::Vector2d v, Math::Vector2d w);

template<typename T>
int find(Common::Array<T>& array, const T& o) {
	int index = -1;
	for (int i = 0; i < array.size(); i++) {
		if (array[i] == o) {
			index = i;
			break;
		}
	}
	return index;
}

} // namespace Twp

#endif
