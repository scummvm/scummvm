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

#include "titanic/support/rect.h"

namespace Titanic {

void Rect::combine(const Rect &r) {
	if (isEmpty()) {
		*this = r;
	} else if (!r.isEmpty()) {
		Common::Rect::extend(r);
	}
}

void Rect::constrain(const Rect &r) {
	if (!isEmpty()) {
		if (r.isEmpty()) {
			clear();
		} else {
			Common::Rect::clip(r);
		}
	}
}

Point Rect::getPoint(Quadrant quadrant) {
	if (isEmpty())
		return Point(left, top);

	switch (quadrant) {
	case Q_LEFT:
		return Point(MIN(left + 10, (int)right), (top + bottom) / 2);
	case Q_RIGHT:
		return Point(MAX(right - 10, (int)left), (top + bottom) / 2);
	case Q_TOP:
		return Point((left + right) / 2, MIN(top + 10, (int)bottom));
	case Q_BOTTOM:
		return Point((left + right) / 2, MAX(bottom - 10, (int)top));
	default:
		return Point((left + right) / 2, (top + bottom) / 2);
	}
}

} // End of namespace Titanic
