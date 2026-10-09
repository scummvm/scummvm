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

#ifndef ULTIMA3_DATA_SAVEGAME_H
#define ULTIMA3_DATA_SAVEGAME_H

#include "ultima/ultima3/data/roster.h"

namespace Ultima {
namespace Ultima3 {
namespace Data {

constexpr byte LOCATION_SOSARIA = 0;
constexpr byte LOCATION_DUNGEON = 1;
constexpr byte LOCATION_TOWN = 2;
constexpr byte LOCATION_CASTLE = 3;

/**
 * The persistent game state: the roster of up to 20 characters, and which
 * of them (by 1-based roster number) make up the active party.
 * Party members are referenced in place rather than copied.
 */
struct Savegame {
	RosterEntry _roster[ROSTER_COUNT];
	byte _partyEntries[PARTY_MAX] = {};
	int _partySize = 0;
	byte _transport = 0;
	byte _location = 0;
	byte _posX = 0;
	byte _posY = 0;

	// Where the party was on the overworld, while they're somewhere else
	byte _worldX = 0;
	byte _worldY = 0;
	byte _moveCount[4] = {};

	// Set once the party has seen the more powerful gear offered in the
	// shops, making it available for readying and wearing
	bool _allWeapons = false;
	bool _allArmour = false;
	bool _plusTwoWeapons = false;
	bool _plusTwoArmour = false;

	// Set once the world map has been loaded, which the map in a save then replaces
	bool _mapLoaded = false;

	/**
	 * Returns a roster entry by its 1-based number
	 */
	RosterEntry &entry(int number) {
		return _roster[number - 1];
	}

	RosterEntry &partyMember(int slot) {
		return entry(_partyEntries[slot]);
	}

	/**
	 * Returns true if any party member is in a condition to adventure
	 */
	bool hasLivingPartyMember();

	/**
	 * Adds a roster entry to the party being selected
	 */
	void addToParty(int number);

	/**
	 * Undoes a partially selected party
	 */
	void clearPartySelection();

	void synchronize(Common::Serializer &s);

	/**
	 * Completes a selected party, placing it at the starting point on foot
	 */
	void formParty();
};

} // namespace Data
} // namespace Ultima3
} // namespace Ultima

#endif
