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

#include "engines/myst3/hotspot.h"
#include "engines/myst3/state.h"

#include "common/config-manager.h"

namespace Myst3 {

HotSpot::HotSpot() :
		condition(0),
		cursor(0) {
}

int32 HotSpot::isPointInRectsCube(float pitch, float heading) {
	for (uint j = 0; j < rects.size(); j++) {
		Common::Rect rect = Common::Rect(
				rects[j].centerHeading - rects[j].width / 2,
				rects[j].centerPitch - rects[j].height / 2,
				rects[j].centerHeading + rects[j].width / 2,
				rects[j].centerPitch + rects[j].height / 2);

		// Make sure heading is in the correct range
		if (rect.right > 360 && heading <= rect.right - 360)
			heading += 360;

		if (pitch > rect.bottom || pitch < rect.top) {
			// Pitch not in rect
			continue;
		}

		if (heading > rect.right || heading < rect.left) {
			// Heading not in rect
			continue;
		}

		// Point in rect
		return j;
	}

	return -1;
}

int32 HotSpot::isPointInRectsFrame(GameState *state, const Common::Point &p) {
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
			return j;
	}

	return -1;
}

bool HotSpot::isEnabled(GameState *state, uint16 var) {
	if (!state->evaluate(condition))
		return false;

	if (isZip()) {
		if (!ConfMan.getBool("zip_mode") || !isZipDestinationAvailable(state)) {
			return false;
		}
	}

	if (var == 0)
		return cursor <= 13;
	else
		return cursor == var;
}

int32 HotSpot::isZipDestinationAvailable(GameState *state) {
	assert(isZip() && script.size() != 0);

	uint16 node;
	uint16 room;

	// Get the zip destination from the script
	Opcode op = script[0];
	switch (op.op) {
	case 140:
	case 142:
		node = op.args[0];
		room = 0;
		break;
	case 141:
	case 143:
		node = op.args[1];
		room = op.args[0];
		break;
	default:
		error("Expected zip action");
	}

	return state->isZipDestinationAvailable(node, room);
}

} // End of namespace Myst3
