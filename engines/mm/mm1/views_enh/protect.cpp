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

#include "mm/mm1/views_enh/protect.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

Protect::Protect() : ScrollView("Protect") {
	setBounds(Common::Rect(0, 0, 320, 200));
	addButton(&g_globals->_escSprites, Common::Point(134, 170), 0, KEYBIND_ESCAPE, true);
}

void Protect::draw() {
	ScrollView::draw();

	setReduced(false);
	writeLine(0, STRING["dialogs.protect.title"], ALIGN_MIDDLE);
	writeString(152, 172, STRING["enhdialogs.misc.go_back"]);

	setReduced(true);
	int yp = 4;

	// Protection spells
	for (int i = 0; i < 7; ++i) {
		if (g_globals->_activeSpells._arr[i]) {
			Common::String line = Common::String::format("%s %s + %d%%",
				STRING["dialogs.protect.protection"].c_str(),
				STRING[Common::String::format("dialogs.protect.%d", i + 1)].c_str(),
				g_globals->_activeSpells._arr[i]);
			writeString(0, yp += 8, line, ALIGN_MIDDLE);
		}
	}

	// Light spell
	if (g_globals->_activeSpells._arr[7]) {
		writeString(0, yp += 8, Common::String::format("%s%d)",
			STRING["dialogs.protect.8"].c_str(),
			g_globals->_activeSpells._arr[7]), ALIGN_MIDDLE);
	}

	// The rest
	for (int i = 8; i < 18; ++i) {
		if (i == 13)
			yp += 8;

		if (g_globals->_activeSpells._arr[i]) {
			Common::String line = STRING[Common::String::format(
				"dialogs.protect.%d", i + 1)];
			if (i == 17) {
				line += Common::String::format("%d %s",
					g_globals->_activeSpells._arr[17],
					STRING["dialogs.protect.to_attacks"].c_str()
				);
			}

			writeString(0, yp += 8, line, ALIGN_MIDDLE);
		}
	}
}

bool Protect::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case KEYBIND_ESCAPE:
		close();
		return true;
	default:
		break;
	}

	return false;
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
