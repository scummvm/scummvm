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

#ifndef ULTIMA3_LOGIC_COMBAT_LOGIC_H
#define ULTIMA3_LOGIC_COMBAT_LOGIC_H

#include "common/array.h"
#include "common/str.h"
#include "ultima/ultima3/data/arena.h"
#include "ultima/ultima3/logic/logic.h"

namespace Ultima {
namespace Ultima3 {

/**
 * One thing to show or do as a fight plays out. The rules are carried out
 * as soon as a command is given, and these are then played back a frame at
 * a time so what's seen matches the order things happened in
 */
struct CombatEvent {
	enum Type {
		PRINT,			// Adds text to the message window
		SOUND,			// Plays a sound effect
		OVERLAY,		// Shows a tile over the arena for a moment
		FLASH_SLOT,		// Inverts a party member's panel for a moment
		FLASH_VIEW,		// Inverts the arena for a moment
		SHOW			// Brings what's shown up to date with the fight
	};

	Type _type;
	int _arg1 = 0, _arg2 = 0, _arg3 = 0;
	Common::String _text;

	CombatEvent(Type type, int a = 0, int b = 0, int c = 0) : _type(type), _arg1(a), _arg2(b), _arg3(c) {}
	CombatEvent(const Common::String &text) : _type(PRINT), _text(text) {}
};

/**
 * A fight between the party and a group of monsters on a small arena
 */
class CombatLogic : public Logic {
private:
	// What's drawn on the arena, which lags behind the real state while the
	// events for a command are being played
	byte _shown[Data::ARENA_SIZE * Data::ARENA_SIZE] = {};

	Common::Array<CombatEvent> _events;
	int _negateTurns = 0;

	void print(const Common::String &text);
	void sound(int effect);
	void overlay(int x, int y, int tile);
	void flashSlot(int slot);
	void show();

	/**
	 * Returns the file holding the arena for a fight
	 */
	const char *chooseArena();

	/**
	 * Returns a number from 0 up to but not including a limit
	 */
	static int rollBelow(int limit);

	/**
	 * Returns true if the fight is one of those within Exodus's castle,
	 * where only the strongest weapons work
	 */
	bool isExodusFight() const;

	/**
	 * Returns the name of the monsters being fought
	 * @param heading	True for the heading shown as a fight starts, which has them in the plural
	 */
	Common::String monsterName(bool heading) const;

	int monsterAt(int x, int y) const;
	bool canMonsterMoveTo(int x, int y) const;

	/**
	 * Follows a shot from a position until it leaves the arena or reaches a monster
	 * @returns		The monster hit, or -1
	 */
	int shoot(int x, int y, int dx, int dy, int tile);

	/**
	 * Works out whether a player hits with the weapon they're using, and
	 * if so how much damage is done to a monster
	 */
	void playerHit(int slot, int monster);
	void damageMonster(int slot, int monster, int damage);

	/**
	 * Finds the nearest party member a monster can head toward
	 * @returns		How far away they are, or zero if it's already beside one
	 *		of the party, in which case the target is that member
	 */
	int findTarget(int monster, int &target, int &stepX, int &stepY, int &toX, int &toY) const;

	void monsterTurn(int monster);
	void monsterAttack(int slot, int tile);
	void monsterPoison(int slot);
	void monsterSteal(int slot);
	void monsterBreath(int monster, int stepX, int stepY);
	void damagePlayer(int slot, int tile);
	void killPlayer(int slot);

public:
	Data::Arena _arena;

	// The tile number of the monsters being fought
	byte _monsterClass = 0;

	// Where the party was when the fight began
	byte _savedLocation = 0;

	int _combatant = 0;

	/**
	 * Starts a fight with one of the creatures on the current map
	 */
	void begin(int creature);

	/**
	 * Brings what's shown up to date with the fight
	 */
	void syncShown();

	/**
	 * Returns the tile to show at a place in the arena
	 */
	byte shownTile(int x, int y) const {
		return _shown[y * Data::ARENA_SIZE + x];
	}

	/**
	 * Returns the events waiting to be played back
	 */
	Common::Array<CombatEvent> &events() {
		return _events;
	}

	bool allMonstersDead() const;

	/**
	 * Returns how many turns of time being held still are left
	 */
	int negateTurns() const {
		return _negateTurns;
	}

	void holdTime() {
		_negateTurns = 10;
	}

	void decrementHold() {
		--_negateTurns;
	}

	void endHold() {
		_negateTurns = 0;
	}

	/**
	 * Lets a party member step in a direction
	 * @returns		False if something is in the way
	 */
	bool movePlayer(int slot, int dx, int dy);

	/**
	 * Has a party member attack in a direction with the weapon they're using
	 */
	void playerAttack(int slot, Direction dir);

	/**
	 * Lets every monster take its turn
	 */
	void monstersTurn();
};

} // namespace Ultima3
} // namespace Ultima

#endif
