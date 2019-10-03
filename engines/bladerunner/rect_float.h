/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BLADERUNNER_RECT_FLOAT_H
#define BLADERUNNER_RECT_FLOAT_H

#include "common/debug.h"
#include "common/math.h"
#include "common/types.h"
#include "common/util.h"

namespace BladeRunner {

struct RectFloat {
	float x0;
	float y0;
	float x1;
	float y1;

	RectFloat()
		: x0(0.0f), y0(0.0f), x1(0.0f), y1(0.0f)
	{}
	RectFloat(float x0_, float y0_, float x1_, float y1_)
		: x0(x0_), y0(y0_), x1(x1_), y1(y1_)
	{}

	void expand(float d) {
		x0 -= d;
		y0 -= d;
		x1 += d;
		y1 += d;
	}

	void trunc_2_decimals() {
		x0 = Common::trunc(x0 * 100.0f) / 100.0f;
		y0 = Common::trunc(y0 * 100.0f) / 100.0f;
		x1 = Common::trunc(x1 * 100.0f) / 100.0f;
		y1 = Common::trunc(y1 * 100.0f) / 100.0f;
	}
};

inline bool overlaps(const RectFloat &a, const RectFloat &b) {
	return !(a.y1 < b.y0 || a.y0 > b.y1 || a.x0 > b.x1 || a.x1 < b.x0);
}

inline RectFloat merge(const RectFloat &a, const RectFloat &b) {
	RectFloat c;
	c.x0 = MIN(a.x0, b.x0);
	c.y0 = MIN(a.y0, b.y0);
	c.x1 = MAX(a.x1, b.x1);
	c.y1 = MAX(a.y1, b.y1);
	return c;
}

} // End of namespace BladeRunner

#endif
