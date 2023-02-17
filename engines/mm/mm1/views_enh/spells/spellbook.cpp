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

#include "mm/mm1/views_enh/spells/spellbook.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Spells {

Spellbook::Spellbook() : ScrollView("Spellbook") {
	_bounds = Common::Rect(27, 6, 195, 142);
}

bool Spellbook::msgFocus(const FocusMessage &msg) {
	g_events->send(GameMessage("CHAR_HIGHLIGHT", (int)true));
	MetaEngine::setKeybindingMode(KeybindingMode::KBMODE_PARTY_MENUS);
	return true;
}

bool Spellbook::msgUnfocus(const UnfocusMessage &msg) {
	// Turn off highlight for selected character
	g_events->send(GameMessage("CHAR_HIGHLIGHT", (int)false));

	MetaEngine::setKeybindingMode(KeybindingMode::KBMODE_MENUS);
	return true;
}

void Spellbook::draw() {
	ScrollView::draw();
	const Character &c = *g_globals->_currCharacter;

	_fontReduced = true;
	Common::String title = Common::String::format("%s %s",
		STRING["enhdialogs.spellbook.title"].c_str(),
		c._name
	);
	writeString(0, 0, title, ALIGN_MIDDLE);
}

bool Spellbook::msgKeypress(const KeypressMessage &msg) {
	return false;
}

bool Spellbook::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		close();
		return true;

	} else if (msg._action >= KEYBIND_VIEW_PARTY1 &&
		msg._action <= KEYBIND_VIEW_PARTY6 && !g_events->isInCombat()) {
		uint charNum = msg._action - KEYBIND_VIEW_PARTY1;

		if (charNum < g_globals->_party.size())
			selectChar(charNum);
		return true;
	}

	return false;
}

void Spellbook::selectChar(uint charNum) {
	assert(!g_events->isInCombat());
	g_globals->_currCharacter = &g_globals->_party[charNum];

	// Refresh the cast spell side dialog for new character
	send("CastSpell", GameMessage("UPDATE"));

	// Update the highlighted char in the party display
	g_events->send(GameMessage("CHAR_HIGHLIGHT", (int)true));

	// And finally, update the display
	redraw();
}

void selectedCharChanged() {

}

void Spellbook::updateSelectedSpell() {
	/*
	const Character &c = *g_globals->_currCharacter;

	if (c._nonCombatSpell == -1) {
		_requiredSp = _requiredGems = 0;

	} else {
		int lvl, num;
		getSpellLevelNum(c._nonCombatSpell, lvl, num);
		assert(getSpellIndex(&c, lvl, num) == c._nonCombatSpell);

		setSpell(&c, lvl, num);
	}
	*/
}

} // namespace Spells
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
