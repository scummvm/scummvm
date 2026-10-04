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

#ifndef ULTIMA3_DATA_ROSTER_H
#define ULTIMA3_DATA_ROSTER_H

#include "common/scummsys.h"
#include "common/serializer.h"

namespace Ultima {
namespace Ultima3 {
namespace Data {

constexpr int ROSTER_COUNT = 20;
constexpr int PARTY_MAX = 4;
constexpr int NAME_MAX = 9;

constexpr byte IN_PARTY = 0xFF;
constexpr byte STATUS_GOOD = 'G';
constexpr byte STATUS_POISONED = 'P';
constexpr byte STATUS_DEAD = 'D';
constexpr byte STATUS_ASHES = 'A';

/**
 * Converts a value of 0-99 to packed BCD, the form the original stores
 * attributes, hit points and so on in
 */
inline byte toBcd(int value) {
	return ((value / 10) << 4) | (value % 10);
}

inline uint16 toBcdWord(int value) {
	return (toBcd(value / 100) << 8) | toBcd(value % 100);
}

inline int fromBcd(byte value) {
	return (value >> 4) * 10 + (value & 0xF);
}

inline int fromBcdWord(uint16 value) {
	return fromBcd(value >> 8) * 100 + fromBcd(value & 0xFF);
}

/**
 * A character record. Numeric fields hold BCD values, exactly as in the
 * original, so the same digits are shown when they're printed as hex.
 */
struct RosterEntry {
	char _name[NAME_MAX + 1] = {};
	byte _marksAndCards = 0;
	byte _torches = 0;
	byte _partyMember = 0;
	byte _status = 0;
	byte _strength = 0;
	byte _dexterity = 0;
	byte _intelligence = 0;
	byte _wisdom = 0;
	byte _race = 0;
	byte _class = 0;
	byte _sex = 0;
	byte _magicPoints = 0;
	uint16 _hitPoints = 0;
	uint16 _maxHitPoints = 0;
	uint16 _experience = 0;
	byte _foodSubCounter = 0;
	uint16 _food = 0;
	uint16 _gold = 0;
	byte _gems = 0;
	byte _keys = 0;
	byte _powder = 0;
	byte _armourIndex = 0;
	byte _armourOwned[7] = {};
	byte _weaponIndex = 0;
	byte _weaponOwned[15] = {};

	void clear() {
		*this = RosterEntry();
	}

	bool isEmpty() const {
		return _name[0] == '\0';
	}

	bool isInParty() const {
		return _partyMember == IN_PARTY;
	}

	bool isAlive() const {
		return _status == STATUS_GOOD || _status == STATUS_POISONED;
	}

	void synchronize(Common::Serializer &s);
};

} // namespace Data
} // namespace Ultima3
} // namespace Ultima

#endif
