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

#ifndef GLK_UTILS_H
#define GLK_UTILS_H

#include "common/rect.h"
#include "glk/glk_types.h"

namespace Gargoyle {

typedef Common::Point Point;

struct Rect : public Common::Rect {
public:
	static Rect fromXYWH(int x, int y, int w, int h) {
		return Rect(x, y, x + w, y + h);
	}

	Rect() : Common::Rect() {}
	Rect(int16 w, int16 h) : Common::Rect(w, h) {}
	Rect(int16 x1, int16 y1, int16 x2, int16 y2) : Common::Rect(x1, y1, x2, y2) {}
};

/**
 * Converts a decimal or hexadecimal string into a number
 */
int strToInt(const char *s);

} // End of namespace Gargoyle

#endif
