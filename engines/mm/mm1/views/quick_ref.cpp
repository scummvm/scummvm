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

#include "mm/mm1/views/quick_ref.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Views {

bool QuickRef::msgFocus(const FocusMessage &msg) {
	MetaEngine::setKeybindingMode(KeybindingMode::KBMODE_PARTY_MENUS);
	return TextView::msgFocus(msg);
}

bool QuickRef::msgUnfocus(const UnfocusMessage &msg) {
	MetaEngine::setKeybindingMode(KeybindingMode::KBMODE_MENUS);
	return TextView::msgUnfocus(msg);
}

void QuickRef::draw() {
	clearSurface();
	writeString(STRING["dialogs.quick_ref.title"]);

	// Print list of characters, hit pts, spell pts, and ac
	size_t partySize = getPartySize();
	bool inCombat = isInCombat();

	for (uint idx = 0; idx < partySize; ++idx) {
		Character &c = inCombat ? *g_globals->_combatParty[idx] :
			g_globals->_party[idx];

		// Number and name
		writeNumber(0, 2 + idx, idx + 1);
		_textPos.x++;
		writeString(c._name);

		// Hit points
		_textPos.x = 18;
		writeNumber(c._hp);
		_textPos.x = 22;
		writeChar('/');
		writeNumber(c._hpMax);

		// Spell points
		_textPos.x = 28;
		writeNumber(c._sp._current);
		_textPos.x = 32;
		writeChar('/');
		writeNumber(c._sp._base);

		// AC
		_textPos.x = 38;
		writeNumber(c._ac);
	}

	// Print food and conditions of each character
	for (uint idx = 0; idx < g_globals->_party.size(); ++idx) {
		Character &c = inCombat ? *g_globals->_combatParty[idx] :
			g_globals->_party[idx];
		writeNumber(0, 9 + idx, idx + 1);

		_textPos.x++;
		writeString(STRING["dialogs.quick_ref.food"]);
		writeNumber(c._food);

		_textPos.x = 12;
		printCondition();
	}

	writeString(12, 21, "'1'-'");
	writeNumber(g_globals->_party.size());
	writeString("' ");
	writeString(STRING["dialogs.quick_ref.to_view"]);

	escToGoBack();
}

bool QuickRef::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode == Common::KEYCODE_ESCAPE) {
		close();
		return true;
	}

	return false;
}

bool QuickRef::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case KEYBIND_VIEW_PARTY1:
	case KEYBIND_VIEW_PARTY2:
	case KEYBIND_VIEW_PARTY3:
	case KEYBIND_VIEW_PARTY4:
	case KEYBIND_VIEW_PARTY5:
	case KEYBIND_VIEW_PARTY6: {
		uint charNum = msg._action - KEYBIND_VIEW_PARTY1;
		if (charNum < getPartySize()) {
			if (isInCombat()) {
				g_globals->_currCharacter = g_globals->_combatParty[charNum];
				replaceView("CharacterViewCombat");
			} else {
				g_globals->_currCharacter = &g_globals->_party[charNum];
				replaceView("CharacterInfo");
			}
		}
		break;
	}
	default:
		break;
	}

	return false;
}

bool QuickRef::isInCombat() const {
	return g_events->isPresent("Combat");
}

size_t QuickRef::getPartySize() const {
	return g_globals->_party.size();
}

} // namespace Views
} // namespace MM1
} // namespace MM
