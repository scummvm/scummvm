/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/myst3/hotspot.h"
#include "engines/myst3/state.h"

namespace Myst3 {

bool HotSpot::isPointInRectsCube(const Common::Point &p) {
	for (uint j = 0; j < rects.size(); j++) {
		Common::Rect rect = Common::Rect(
				rects[j].centerHeading - rects[j].width / 2,
				rects[j].centerPitch - rects[j].height / 2,
				rects[j].centerHeading + rects[j].width / 2,
				rects[j].centerPitch + rects[j].height / 2);
		if (rect.contains(p))
			return true;
	}

	return false;
}

bool HotSpot::isPointInRectsFrame(GameState *state, const Common::Point &p) {
	for (uint j = 0; j < rects.size(); j++) {

		int16 x = rects[j].centerPitch;
		int16 y = rects[j].centerHeading;
		int16 w = rects[j].width;
		int16 h = rects[j].height;

		if (y < 0) {
			x = state->getVar(x);
			y = state->getVar(-y);
			h = -h;
		}

		Common::Rect rect = Common::Rect(w, h);
		rect.translate(x, y);
		if (rect.contains(p))
			return true;
	}

	return false;
}

} /* namespace Myst3 */
