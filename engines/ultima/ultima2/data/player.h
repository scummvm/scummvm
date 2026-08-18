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

#ifndef ULTIMA2_DATA_PLAYER_H
#define ULTIMA2_DATA_PLAYER_H

#include "common/serializer.h"

namespace Ultima {
namespace Ultima2 {
namespace Data {

constexpr int MAX_NAME_LENGTH = 12;

extern const char *SEX[];
extern const char *RACE[];
extern const char *CLASS[];

enum Sex {
	SEX_MALE, SEX_FEMALE
};
enum Race {
	RACE_HUMAN = 1, RACE_ELF = 2, RACE_DWARF = 3, RACE_HOBBIT = 4
};
enum Class {
	CLASS_FIGHTER = 1, CLASS_CLERIC = 2, CLASS_WIZARD = 3, CLASS_THIEF = 4
};

struct Player {
private:
	/**
	 * Synchronizes a byte value. In original Ultima II, byte values store
	 * the tens in the upper 4 bits, and the 0-9 portion in the low 4 bits.
	 */
	void syncByte(Common::Serializer &s, byte &v);

	/**
	 * Synchronizes a uint16 value. In original Ultima II, uint16 values store
	 * 4 digits each in 4-bit portions of their own.
	 */
	void syncWord(Common::Serializer &s, uint16 &v);

public:
	char _name[MAX_NAME_LENGTH + 1] = {};
	byte _sex = SEX_MALE;
	byte _class = CLASS_FIGHTER;
	byte _race = RACE_HUMAN;
	byte _mapNum = 20;

	byte _strength = 0;
	byte _agility = 0;
	byte _stamina = 0;
	byte _charisma = 0;
	byte _wisdom = 0;
	byte _intelligence = 0;
	uint16 _hp = 400;
	uint16 _food = 400;
	byte _foodSubCtr = 87;
	uint16 _experience = 0;
	uint16 _gold = 400;

	byte _mapX = 20;
	byte _mapY = 20;
	byte _field3E = 0xff;

	bool _hasRing = false;

	// Runtime only fields
	byte _paralyzedFlag = 0;

	void synchronize(Common::Serializer &s);
	bool isDead() const {
		return _hp > 0;
	}
};

} // namespace Data
} // namespace Ultima2
} // namespace Ultima

#endif
