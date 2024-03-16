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

#ifndef TWP_RECTF_H
#define TWP_RECTF_H

#include "common/array.h"
#include "math/vector2d.h"

namespace Twp {

struct Rectf {
public:
	Rectf();
	Rectf(float x, float y, float w, float h);

	static Rectf fromPosAndSize(Math::Vector2d pos, Math::Vector2d size);
	static Rectf fromMinMax(Math::Vector2d min, Math::Vector2d max);

	Rectf operator/(Math::Vector2d v);

	union {
		float v[4];
		struct {
			float x;
			float y;
			float w;
			float h;
		} r;
	};

	inline float left() { return r.x; }
	inline float right() { return r.x + r.w; }
	inline float top() { return r.y + r.h; }
	inline float bottom() { return r.y; }

	bool contains(Math::Vector2d pos) {
		return pos.getX() >= r.x && pos.getX() <= (r.x + r.w) && pos.getY() >= r.y && pos.getY() <= r.y + r.h;
	}
};
} // namespace Twp

#endif
