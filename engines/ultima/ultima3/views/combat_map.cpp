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

#include "ultima/ultima3/views/combat_map.h"
#include "ultima/ultima3/views/menu_input.h"
#include "ultima/ultima3/views/interactions/cast_spell.h"
#include "ultima/ultima3/views/interactions/combat_attack.h"
#include "ultima/ultima3/views/interactions/equip.h"
#include "ultima/ultima3/views/interactions/ztats.h"
#include "ultima/ultima3/data/data.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

constexpr int ARENA_LEFT = 8;
constexpr int ARENA_TOP = 8;

bool CombatMap::msgFocus(const FocusMessage &msg) {
	if (!_started) {
		_started = true;
		_next = NOTHING;
		_overlay = false;
		beginRound();
	}

	return Game::msgFocus(msg);
}

void CombatMap::beginRound() {
	_G(combat).processPartyTurnEffects(false);
	_G(combat)._combatant = 0;
	beginTurn();
}

void CombatMap::beginTurn() {
	CombatLogic &combat = _G(combat);
	Data::Savegame &save = _G(savegame);

	for (; combat._combatant < save._partySize; ++combat._combatant) {
		_G(effects)._highlight = combat._combatant;

		if (save.partyMember(combat._combatant).isAlive()) {
			_G(messages).print(Common::String::format("----Player %d----\n", combat._combatant + 1).c_str());
			startPrompt();
			return;
		}

		combat.incrementMoveCounter(1);
	}

	_G(effects)._highlight = -1;
	monstersTurn();
}

void CombatMap::advanceTurn() {
	CombatLogic &combat = _G(combat);

	combat.incrementMoveCounter(1);
	_G(effects)._highlight = -1;

	if (combat.allMonstersDead()) {
		victory();
		return;
	}

	++combat._combatant;
	beginTurn();
}

void CombatMap::monstersTurn() {
	CombatLogic &combat = _G(combat);

	if (combat.negateTurns() > 0)
		combat.decrementHold();
	else
		combat.monstersTurn();

	_next = NEXT_ROUND;
}

void CombatMap::victory() {
	_G(messages).print("****Victory!****\n\n");
	g_engine->playSoundEffect(0xFD);

	_G(combat).endHold();
	_G(effects)._highlight = -1;
	_started = false;
	close();
}

void CombatMap::endTurn() {
	_next = NEXT_PLAYER;
}

bool CombatMap::isWaiting() const {
	return _next == NOTHING && _G(combat).events().empty();
}

void CombatMap::processFrame() {
	_overlay = false;
	Common::Array<CombatEvent> &events = _G(combat).events();

	while (!events.empty()) {
		CombatEvent event = events[0];
		events.remove_at(0);

		switch (event._type) {
		case CombatEvent::PRINT:
			_G(messages).print(event._text.c_str());
			break;
		case CombatEvent::SOUND:
			g_engine->playSoundEffect(event._arg1);
			break;
		case CombatEvent::OVERLAY:
			_overlay = true;
			_overlayX = event._arg1;
			_overlayY = event._arg2;
			_overlayTile = event._arg3;
			return;
		case CombatEvent::FLASH_SLOT:
			_G(effects).flashSlot(event._arg1);
			break;
		case CombatEvent::FLASH_VIEW:
			_G(effects).flashViewport();
			return;
		case CombatEvent::SHOW:
			_G(combat).syncShown();
			return;
		}
	}

	if (_next == NOTHING || _gameOver)
		return;

	if (checkPartyWipedOut()) {
		_next = NOTHING;
		return;
	}

	Next next = _next;
	_next = NOTHING;

	if (next == NEXT_PLAYER)
		advanceTurn();
	else
		beginRound();
}

void CombatMap::idleTimeout() {
	_G(messages).print("Pass\n");
	endTurn();
}

bool CombatMap::msgKeypress(const KeypressMessage &msg) {
	// Keys are ignored while the last command is still playing out
	if (!isWaiting() && !_gameOver)
		return true;

	return Game::msgKeypress(msg);
}

void CombatMap::doMove(int dx, int dy, const char *label) {
	_G(messages).print(label);

	if (!_G(combat).movePlayer(_G(combat)._combatant, dx, dy)) {
		_G(messages).print("Invalid Move!\n");
		g_engine->playSoundEffect(0xFF);
	}

	endTurn();
}

void CombatMap::doNegateTime() {
	Data::RosterEntry &e = _G(savegame).partyMember(_G(combat)._combatant);
	_G(messages).print("Negate Time!\n");

	if (e._powder == 0) {
		_G(messages).print("None Left!\n");
		g_engine->playSoundEffect(0xFE);
	} else {
		e._powder = Data::toBcd(Data::fromBcd(e._powder) - 1);
		_G(holdTime) = 10;
	}

	endTurn();
}

void CombatMap::doAttack() {
	const Data::RosterEntry &e = _G(savegame).partyMember(_G(combat)._combatant);

	_G(messages).print(Data::WEAPON_NAMES[e._weaponIndex]);
	_G(messages).print(" Attack\nDir-");
	startInteraction(new Interactions::CombatAttack(_G(combat)._combatant));
}

bool CombatMap::handleCommand(const KeypressMessage &msg) {
	const int slot = _G(combat)._combatant;

	switch (msg.keycode) {
	case Common::KEYCODE_UP:
	case Common::KEYCODE_KP8:
		doMove(0, -1, "North\n");
		return true;
	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_KP2:
		doMove(0, 1, "South\n");
		return true;
	case Common::KEYCODE_RIGHT:
	case Common::KEYCODE_KP6:
		doMove(1, 0, "East\n");
		return true;
	case Common::KEYCODE_LEFT:
	case Common::KEYCODE_KP4:
		doMove(-1, 0, "West\n");
		return true;
	case Common::KEYCODE_SPACE:
		_G(messages).print("Pass\n");
		endTurn();
		return true;
	default:
		break;
	}

	switch (commandKey(msg)) {
	case 'R':
		_G(messages).print("Ready a weapon!\n");
		startInteraction(new Interactions::Equip(true, slot));
		return true;
	case 'Z':
		_G(messages).print("Ztats\n");
		startInteraction(new Interactions::Ztats(slot));
		return true;
	case 'N':
		doNegateTime();
		return true;
	case 'C':
		_G(messages).print("Cast Spell!\n");
		startInteraction(new Interactions::CastSpell(slot));
		return true;
	case 'A':
		doAttack();
		return true;
	case 'V':
		return Game::handleCommand(msg);

	// Keys for commands that can't be used in a fight don't take a turn
	case 'U': case 'B': case 'D': case 'E': case 'F': case 'G': case 'I':
	case 'K': case 'P': case 'Q': case 'S': case 'T': case 'W': case 'X':
	case 'H': case 'J': case 'L': case 'M': case 'O': case 'Y':
		_G(messages).print("Not usable cmd!\n");
		g_engine->playSoundEffect(0xFE);
		startPrompt();
		return true;

	default:
		_G(messages).print("<-What?\n");
		g_engine->playSoundEffect(0xFE);
		endTurn();
		return true;
	}
}

void CombatMap::drawViewport(GfxSurface &s) {
	const CombatLogic &combat = _G(combat);

	for (int y = 0; y < Data::ARENA_SIZE; ++y) {
		for (int x = 0; x < Data::ARENA_SIZE; ++x) {
			byte tile = (_overlay && x == _overlayX && y == _overlayY) ? _overlayTile : combat.shownTile(x, y);

			_G(shapes).drawTile(s, ARENA_LEFT + x * Gfx::SHAPE_SIZE, ARENA_TOP + y * Gfx::SHAPE_SIZE, tile);
		}
	}

	s.addDirtyRect(Common::Rect(ARENA_LEFT, ARENA_TOP, ARENA_LEFT + Data::ARENA_SIZE * Gfx::SHAPE_SIZE,
		ARENA_TOP + Data::ARENA_SIZE * Gfx::SHAPE_SIZE));
}

} // namespace Views
} // namespace Ultima3
} // namespace Ultima
