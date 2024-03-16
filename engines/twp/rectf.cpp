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

#include "twp/rectf.h"

namespace Twp {

Rectf::Rectf() : Rectf(0, 0, 0, 0) {}

Rectf::Rectf(float x, float y, float w, float h) {
	r.x = x;
	r.y = y;
	r.w = w;
	r.h = h;
}

Rectf Rectf::fromPosAndSize(Math::Vector2d pos, Math::Vector2d size) {
	return {pos.getX(), pos.getY(), size.getX(), size.getY()};
}

Rectf Rectf::fromMinMax(Math::Vector2d min, Math::Vector2d max) {
	return {min.getX(), min.getY(), max.getX() - min.getX() + 1, max.getY() - min.getY() + 1};
}

Rectf Rectf::operator/(Math::Vector2d nv) {
	return Rectf(r.x / nv.getX(), r.y / nv.getY(), r.w / nv.getX(), r.h / nv.getY());
}

} // namespace Twp
