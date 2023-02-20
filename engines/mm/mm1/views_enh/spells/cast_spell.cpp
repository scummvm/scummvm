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

#include "mm/mm1/views_enh/spells/cast_spell.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Spells {

CastSpell::CastSpell() : PartyView("CastSpell") {
	_bounds = Common::Rect(225, 0, 320, 146);

	_icons.load("cast.icn");
	addButton(&_icons, Common::Point(0, 100), 0,
		Common::KeyState(Common::KEYCODE_c, 'c'));
	addButton(&_icons, Common::Point(28, 100), 2,
		Common::KeyState(Common::KEYCODE_n, 'n'));
	addButton(&_icons, Common::Point(56, 100), 4, KEYBIND_ESCAPE);
}

bool CastSpell::msgFocus(const FocusMessage &msg) {
	(void)PartyView::msgFocus(msg);
	updateSelectedSpell();
	return true;
}

void CastSpell::draw() {
	ScrollView::draw();
	_fontReduced = false;

	const Character &c = *g_globals->_currCharacter;
	writeString(0, 0, STRING["enhdialogs.cast_spell.title"], ALIGN_MIDDLE);
	writeString(0, 20, c._name, ALIGN_MIDDLE);
	writeString(0, 40, STRING["enhdialogs.cast_spell.spell_ready"]);

	setTextColor(37);

	Common::String spellName = STRING["enhdialogs.cast_spell.none"];
	if (c._nonCombatSpell >= 0 && c._nonCombatSpell < 47) {
		spellName = STRING[Common::String::format("spells.cleric.%d", c._nonCombatSpell)];
	} else if (c._nonCombatSpell >= 47) {
		spellName = STRING[Common::String::format("spells.wizard.%d", c._nonCombatSpell - 47)];
	}
	writeString(0, 60, spellName, ALIGN_MIDDLE);

	_fontReduced = true;
	setTextColor(0);
	writeString(0, 80, STRING["enhdialogs.cast_spell.cost"]);
	writeString(0, 90, STRING["enhdialogs.cast_spell.cur_sp"]);
	writeString(0, 80, Common::String::format("%d/%d",
		_requiredSp, _requiredGems), ALIGN_RIGHT);
	writeString(0, 90, Common::String::format("%d", c._sp._current), ALIGN_RIGHT);

	writeString(0, 122, STRING["enhdialogs.cast_spell.cast"]);
	writeString(30, 122, STRING["enhdialogs.cast_spell.new"]);
	writeString(60, 122, STRING["enhdialogs.cast_spell.esc"]);

	_fontReduced = false;
}

bool CastSpell::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode == Common::KEYCODE_c) {
		close();

		const Character &c = *g_globals->_currCharacter;
		if (c._nonCombatSpell != -1) {
			// TODO: Cast spell here
		}
		return true;
	} else if (msg.keycode == Common::KEYCODE_n) {
		// Select a new spell
		addView("Spellbook");
		return true;
	} else {
		return PartyView::msgKeypress(msg);
	}
}

bool CastSpell::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		close();
		return true;

	} else {
		return PartyView::msgAction(msg);
	}

	return false;
}

bool CastSpell::msgGame(const GameMessage &msg) {
	if (msg._name == "UPDATE") {
		updateSelectedSpell();
		draw();
		return true;
	}

	return true;
}

void CastSpell::updateSelectedSpell() {
	const Character &c = *g_globals->_currCharacter;

	if (c._nonCombatSpell == -1) {
		_requiredSp = _requiredGems = 0;

	} else {
		int lvl, num;
		getSpellLevelNum(c._nonCombatSpell, lvl, num);
		assert(getSpellIndex(&c, lvl, num) == c._nonCombatSpell);

		setSpell(&c, lvl, num);
	}
}

} // namespace Spells
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
