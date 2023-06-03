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

#include "mm/mm1/views/spells/location.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Spells {

Location::Location() : SpellView("Location") {
	_bounds = getLineBounds(20, 24);
}

void Location::draw() {
	Maps::Maps &maps = *g_maps;
	Maps::Map &map = *maps._currentMap;
	byte v;

	clearSurface();
	escToGoBack(0);

	writeString(0, 0, STRING["dialogs.spells.location_loc"]);

	v = map[37];
	if (v == 0xff) {
		writeString(STRING["dialogs.spells.location_unknown"]);
	} else {
		if (v == 0) {
			writeString(STRING["dialogs.spells.location_outdoors"]);
		} else if (!(v & 0x80)) {
			writeChar('0' + map[37]);
			writeString(STRING["dialogs.spells.location_undoor"]);
		} else if (v == 0xfe) {
			writeString(STRING["dialogs.spells.location_town"]);
		} else {
			writeString(STRING["dialogs.spells.location_castle"]);
		}

		writeString(21, 0, STRING["dialogs.spells.location_sector"]);
		writeChar(map[Maps::MAP_35] & 0x7f);
		writeChar('-');
		writeChar(map[Maps::MAP_36] & 0x7f);

		writeString(21, 1, STRING["dialogs.spells.location_surface_x"]);

		if (map[Maps::MAP_37]) {
			writeNumber(map[Maps::MAP_SURFACE_X]);
			writeString(35, 1, "Y=");
			writeNumber(map[Maps::MAP_SURFACE_Y]);

			writeString(22, 2, STRING["dialogs.spells.location_inside_x"]);
		}

		writeNumber(maps._mapPos.x);
		writeString(35, 2, "Y=");
		writeNumber(maps._mapPos.y);

		writeString(22, 3, STRING["dialogs.spells.location_facing"]);
		switch (maps._forwardMask) {
		case Maps::DIRMASK_N:
			writeChar('N');
			break;
		case Maps::DIRMASK_S:
			writeChar('S');
			break;
		case Maps::DIRMASK_E:
			writeChar('E');
			break;
		default:
			writeChar('W');
			break;
		}
	}
}

bool Location::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_SELECT || msg._action == KEYBIND_ESCAPE)
		close();

	return true;
}

} // namespace Spells
} // namespace Views
} // namespace MM1
} // namespace MM
