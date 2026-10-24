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
#include "mm/mm1/data/text.h"
#include "mm/mm1/game/spells_party.h"
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
	if (!isInCombat())
		(void)PartyView::msgFocus(msg);

	updateSelectedSpell();
	return true;
}

bool CastSpell::msgUnfocus(const UnfocusMessage &msg) {
	if (!isInCombat())
		(void)PartyView::msgUnfocus(msg);

	return true;
}

void CastSpell::draw() {
	if (!isInCombat()) {
		PartyView::draw();
	} else {
		ScrollView::draw();
	}
	_fontReduced = false;

	const Character &c = *g_globals->_currCharacter;
	writeString(0, 0, "Cast Spell", ALIGN_MIDDLE);
	writeString(0, 16, c._name, ALIGN_MIDDLE);
	writeString(0, 40, "Spell Ready:");

	setTextColor(37);

	int spellNum = c.spellNumber();
	Common::String spellName = "None Ready";
	if (spellNum >= 0 && spellNum < 47) {
		spellName = CLERIC_SPELL_NAMES[spellNum];

		// MM1 Cleric 2-5 is Protection From Fire.
		if (spellNum == 12)
			spellName = "Protection From Fire";
	} else if (spellNum >= 47) {
		spellName = WIZARD_SPELL_NAMES[spellNum - 47];
	}
	writeString(0, 60, spellName, ALIGN_MIDDLE);

	_fontReduced = true;
	setTextColor(0);
	writeString(0, 80, "Cost");
	writeString(0, 90, "Cur SP");
	writeString(0, 80, Common::String::format("%d/%d",
		_requiredSp, _requiredGems), ALIGN_RIGHT);
	writeString(0, 90, Common::String::format("%d", c._sp._current), ALIGN_RIGHT);

	writeString(0, 122, "\x01""37Cast");
	writeString(30, 122, "\x01""37New");
	writeString(60, 122, "ESC");

	_fontReduced = false;
}

bool CastSpell::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode == Common::KEYCODE_c) {
		// Cast a spell
		if (_spellIndex != -1) {
			if (!canCast()) {
				close();
				spellError();
			} else if (hasCharTarget()) {
				addView("CharacterSelect");
			} else if (hasMonsterTarget()) {
				close();
				send("Combat", GameMessage("SELECT_SPELL_TARGET"));
			} else {
				close();
				castSpell();
			}
		}

		return true;

	} else if (msg.keycode == Common::KEYCODE_n) {
		// Select a new spell
		addView("Spellbook");
		return true;
	} else if (!isInCombat()) {
		return PartyView::msgKeypress(msg);
	} else {
		return false;
	}
}

bool CastSpell::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		close();
		return true;

	} else if (!isInCombat()) {
		return PartyView::msgAction(msg);
	} else {
		return false;
	}
}

bool CastSpell::msgGame(const GameMessage &msg) {
	if (msg._name == "UPDATE") {
		updateSelectedSpell();
		draw();
		return true;
	} else if (msg._name == "CHAR_SELECTED" && msg._value != -1) {
		close();
		castSpell(&g_globals->_party[msg._value]);

	} else if (msg._name == "MONSTER_SELECTED" &&
			msg._value != -1) {
		castSpell();
	}

	return true;
}

void CastSpell::updateSelectedSpell() {
	const Character &c = *g_globals->_currCharacter;

	int spellNum = c.spellNumber();
	if (spellNum == -1) {
		_requiredSp = _requiredGems = 0;
		_spellIndex = -1;

	} else {
		int lvl, num;
		getSpellLevelNum(spellNum, lvl, num);
		assert(getSpellIndex(&c, lvl, num) == spellNum);

		setSpell(&c, lvl, num);
	}
}

void CastSpell::charSwitched(Character *priorChar) {
	PartyView::charSwitched(priorChar);
	updateSelectedSpell();
}

bool CastSpell::hasMonsterTarget() {
	if (!isInCombat())
		return false;

	// Spells that affect a single chosen monster
	switch (_spellIndex) {
	// Cleric
	case 2:		// Blind
	case 10:	// Pain
	case 14:	// Silence
	case 15:	// Suggestion
	case 20:	// Produce Flame
	case 21:	// Produce Frost
	case 46:	// Sun Ray

	// Wizard
	case 49:	// Energy Blast
	case 50:	// Flame Arrow
	case 55:	// Electric Arrow
	case 56:	// Hypnotize
	case 57:	// Identify Monster
	case 62:	// Scare
	case 71:	// Acid Arrow
	case 72:	// Cold Beam
	case 73:	// Feeble Mind
	case 74:	// Freeze
	case 81:	// Finger of Death
	case 85:	// Disintegration
		return true;

	default:
		return false;
	}
}

void CastSpell::castSpell(Character *target) {
	if (_spellIndex == -1)
		return;

	// During combat, results are shown by the Combat view. An exploration
	// InfoMessage would draw the dungeon or outdoor scene over the battle
	auto showResult = [this](const Common::String &msg) {
		if (isInCombat()) {
			Common::String result = spellResultMessage(msg);
			GameMessage gameMsg("SPELL_RESULT", result);
			gameMsg._value = 20 - (result.size() / 2);
			g_events->send("Combat", gameMsg);
		} else {
			g_events->send(InfoMessage(msg));
		}
	};
	Character &c = *g_globals->_currCharacter;
	if (isInCombat()) {
		Common::String spellName = _spellIndex < 47 ?
			CLERIC_SPELL_NAMES[_spellIndex] :
			WIZARD_SPELL_NAMES[_spellIndex - 47];
		if (_spellIndex == 12)
			spellName = "Protection From Fire";
		const Common::String logLine = target ?
			Common::String::format("%s casts %s on %s.", c._name,
				spellName.c_str(), target->_name) :
			Common::String::format("%s casts %s.", c._name,
				spellName.c_str());
		send("Combat", GameMessage("COMBAT_LOG", logLine));
	}

	c._sp._current = MAX((int)c._sp._current - _requiredSp, 0);
	c._gems = MAX((int)c._gems - _requiredGems, 0);

	if (!isMagicAllowed()) {
		showResult("Magic doesn't work here");

	} else {
		// Cast the spell
		Game::SpellResult result = Game::SpellsParty::cast(_spellIndex, target);

		switch (result) {
		case Game::SR_FAILED:
			showResult("Spell failed");
			break;

		case Game::SR_SUCCESS_DONE:
			showResult("Done");
			break;

		default:
			// Spell done, but don't display done message
			break;
		}
	}
}

void CastSpell::spellError() {
	Common::String msg = getSpellError();

	if (isInCombat()) {
		// Keep spell errors inside Combat. A global drawElements() here can
		// redraw the underlying dungeon/outdoor scene over the battle screen.
		send("Combat", GameMessage("SPELL_RESULT", msg, 0));
	} else {
		g_events->drawElements();
		send(SoundMessage(msg, ALIGN_MIDDLE));
	}
}

} // namespace Spells
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
