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

#include "mm/mm1/views_enh/dead.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

bool Dead::msgFocus(const FocusMessage &msg) {
	TextView::msgFocus(msg);
	Sound::sound2(SOUND_4);

	g_globals->_party.clear();
	g_globals->_roster.load();
	g_globals->_activeSpells.clear();

	// Play the Xeen death sound
	g_engine->_sound->playSound("xeenlaff.voc");

	return true;
}

void Dead::draw() {
	ScrollView::draw();
	setReduced(false);

	writeLine(4, STRING["dialogs.dead.1"], ALIGN_MIDDLE);
	writeLine(6, STRING["dialogs.dead.2"], ALIGN_MIDDLE);
	writeLine(8, STRING["dialogs.dead.3"], ALIGN_MIDDLE);
	writeLine(10, STRING["dialogs.dead.4"], ALIGN_MIDDLE);
	writeLine(12, STRING["dialogs.dead.5"], ALIGN_MIDDLE);
	writeLine(14, STRING["dialogs.dead.6"], ALIGN_MIDDLE);
}

bool Dead::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_SELECT) {
		replaceView("MainMenu");
		return true;
	}

	return false;
}

} // namespace Views
} // namespace MM1
} // namespace MM
