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

#include "mm/mm1/views_enh/spells/location.h"
#include "mm/mm1/globals.h"

#define TEXT_X1 160
#define TEXT_X2 195

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Spells {

Location::Location() : ScrollView("Location") {
	setBounds(Common::Rect(0, 144, 234, 200));
	addButton(&g_globals->_escSprites, Common::Point(5, 28), 0, KEYBIND_ESCAPE, true);
}

void Location::draw() {
	Maps::Maps &maps = *g_maps;
	Maps::Map &map = *maps._currentMap;
	byte v;

	ScrollView::draw();

	setReduced(true);
	writeString(20, 30, STRING["enhdialogs.misc.go_back"]);

	writeLine(0, STRING["dialogs.spells.location_loc"], ALIGN_LEFT, 0);

	v = map[Maps::MAP_TYPE];
	if (v == 0xff) {
		writeString(STRING["dialogs.spells.location_unknown"]);
	} else {
		if (v == 0) {
			writeString(STRING["dialogs.spells.location_outdoors"]);
		} else if (!(v & 0x80)) {
			writeChar('0' + map[37]);
			writeString(STRING["dialogs.spells.location_under"]);
		} else if (v == 0xfe) {
			writeString(STRING["dialogs.spells.location_town"]);
		} else {
			writeString(STRING["dialogs.spells.location_castle"]);
		}

		writeLine(0, STRING["dialogs.spells.location_sector"], ALIGN_LEFT, 111);
		writeChar(map[Maps::MAP_SECTOR1] & 0x7f);
		writeChar('-');
		writeChar(map[Maps::MAP_SECTOR2] & 0x7f);

		writeLine(1, STRING["dialogs.spells.location_surface_x"], ALIGN_RIGHT, TEXT_X1);
		writeString("X=");

		if (map[Maps::MAP_TYPE]) {
			writeNumber(map[Maps::MAP_SURFACE_X]);
			writeLine(1, "Y=", ALIGN_LEFT, TEXT_X2);
			writeNumber(map[Maps::MAP_SURFACE_Y]);

			writeLine(2, STRING["dialogs.spells.location_inside_x"], ALIGN_RIGHT, TEXT_X1);
			writeString("X=");
		}

		writeNumber(maps._mapPos.x);
		_textPos.x = TEXT_X2;
		writeString("Y=");
		writeNumber(maps._mapPos.y);

		writeLine(3, STRING["dialogs.spells.location_facing"], ALIGN_RIGHT, TEXT_X1);
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

	setReduced(false);
}

bool Location::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_SELECT || msg._action == KEYBIND_ESCAPE)
		g_events->replaceView("Game", true);

	return true;
}

} // namespace Spells
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
