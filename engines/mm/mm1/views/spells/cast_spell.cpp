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

#include "mm/mm1/views/spells/cast_spell.h"
#include "mm/mm1/game/spells_party.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Spells {

CastSpell::CastSpell() : SpellView("CastSpell") {
	_bounds = getLineBounds(20, 24);
}

bool CastSpell::msgGame(const GameMessage &msg) {
	if (msg._name != "SPELL")
		return false;

	if (msg._value == 0) {
		// Ensure current character can cast spells
		if (g_globals->_currCharacter->_spellLevel != 0 &&
			g_globals->_currCharacter->_sp._current != 0) {
			addView();
			setState(SELECT_SPELL);
		}
	} else {
		// Spell bound to an item
		addView();
		setSpell(msg._value, 0, 0);

		if (!canCast()) {
			spellDone();
		} else if (hasCharTarget()) {
			setState(SELECT_CHAR);
		} else {
			setState(PRESS_ENTER);
		}
	}

	return true;
}

bool CastSpell::msgFocus(const FocusMessage &msg) {
	if (dynamic_cast<TextEntry *>(msg._priorView) == nullptr)
		_state = SELECT_SPELL;

	return true;
}

void CastSpell::setState(State state) {
	_state = state;

	MetaEngine::setKeybindingMode(
		_state == SELECT_CHAR ?
		KeybindingMode::KBMODE_PARTY_MENUS :
		KeybindingMode::KBMODE_MENUS
	);
	draw();
}

void CastSpell::abortFunc() {
	CastSpell *view = (CastSpell *)g_events->focusedView();
	view->close();
}

void CastSpell::enterSpellLevelFunc(const Common::String &text) {
	CastSpell *view = (CastSpell *)g_events->focusedView();
	view->spellLevelEntered(atoi(text.c_str()));
}

void CastSpell::enterSpellNumberFunc(const Common::String &text) {
	CastSpell *view = (CastSpell *)g_events->focusedView();
	view->spellNumberEntered(atoi(text.c_str()));
}

void CastSpell::draw() {
	clearSurface();
	if (_state == NONE)
		return;

	escToGoBack(0);
	writeString(7, 0, STRING["dialogs.character.cast_spell"]);
	if (_state >= SELECT_NUMBER) {
		writeChar(' ');
		writeNumber(_spellLevel);
		writeString(19, 1, STRING["dialogs.character.number"]);
	}

	if (_state > SELECT_NUMBER) {
		writeChar(' ');
		writeNumber(_spellNumber);
	}

	switch (_state) {
	case SELECT_SPELL:
		_state = NONE;
		_textEntry.display(27, 20, 1, true, abortFunc, enterSpellLevelFunc);
		break;

	case SELECT_NUMBER:
		_state = NONE;
		_textEntry.display(27, 21, 1, true, abortFunc, enterSpellNumberFunc);
		break;

	case SELECT_CHAR:
		writeString(22, 3, Common::String::format(
			STRING["spells.cast_on_char"].c_str(),
			(int)g_globals->_party.size()
		));
		break;

	case PRESS_ENTER:
		writeString(24, 4, STRING["spells.enter_to_cast"]);
		break;

	case ENDING:
		clearSurface();
		writeString(_spellResultX, 1, _spellResult);
		delaySeconds(3);
		break;

	default:
		break;
	}
}

void CastSpell::spellLevelEntered(uint level) {
	// Ensure the spell level is valid
	if (level < 1 || level > 7 ||
		(!g_globals->_allSpells && level > g_globals->_currCharacter->_spellLevel)) {
		close();
		return;
	}

	_spellLevel = level;
	setState(SELECT_NUMBER);
}

void CastSpell::spellNumberEntered(uint num) {
	if (num < 1 || num > 8 || (_spellLevel >= 5 && num >= 6)) {
		close();
		return;
	}

	_spellNumber = num;
	setSpell(g_globals->_currCharacter, _spellLevel, num);
	if (!canCast()) {
		spellDone();
	} else {
		if (hasCharTarget())
			setState(SELECT_CHAR);
		else
			setState(PRESS_ENTER);

		draw();
	}
}

bool CastSpell::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		close();

	} else if (msg._action == KEYBIND_SELECT) {
		// Time to execute the spell
		performSpell();

	} else if (_state == SELECT_CHAR &&
		msg._action >= KEYBIND_VIEW_PARTY1 &&
		msg._action <= KEYBIND_VIEW_PARTY6) {
		uint charIndex = (int)(msg._action - KEYBIND_VIEW_PARTY1);
		if (charIndex < g_globals->_party.size()) {
			Character *c = isInCombat() ? g_globals->_combatParty[charIndex] :
				&g_globals->_party[charIndex];
			performSpell(c);
		}
	}

	return true;
}


void CastSpell::timeout() {
	close();
}

void CastSpell::performSpell(Character *chr) {
	Character &c = *g_globals->_currCharacter;
	c._sp._current = MAX((int)c._sp._current - _requiredSp, 0);
	c._gems = MAX((int)c._gems - _requiredGems, 0);

	if (!isMagicAllowed()) {
		spellDone(STRING["spells.magic_doesnt_work"], 5);
	} else {
		// Cast the spell
		switch (Game::SpellsParty::cast(_spellIndex, chr)) {
		case Game::SR_FAILED:
			// Spell failed
			spellFailed();
			break;

		case Game::SR_SUCCESS_DONE:
			// Display spell done
			spellDone();
			break;

		default:
			// Spell done, but don't display done message
			if (isFocused())
				close();
			break;
		}
	}
}

void CastSpell::spellDone() {
	Common::String msg = getSpellError();
	int xp = 20 - (msg.size() / 2);

	spellDone(msg, xp);
}

void CastSpell::spellDone(const Common::String &msg, int xp) {
	if (isInCombat()) {
		close();

		GameMessage gameMsg("SPELL_RESULT", msg);
		gameMsg._value = xp;
		g_events->focusedView()->send(gameMsg);

	} else {
		Sound::sound(SOUND_2);
		_spellResult = msg;
		_spellResultX = xp;
		setState(ENDING);
	}
}

} // namespace Spells
} // namespace Views
} // namespace MM1
} // namespace MM
