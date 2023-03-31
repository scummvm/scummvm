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

#ifndef MM1_DATA_MONSTERS_H
#define MM1_DATA_MONSTERS_H

#include "mm/mm1/gfx/dta.h"
#include "mm/mm1/data/text_parser.h"
#include "common/str.h"
#include "graphics/managed_surface.h"

namespace MM {
namespace MM1 {

#define MONSTERS_COUNT 195

enum MonsterStatus {
	MON_PARALYZED = 0, MON_WEBBED = 1, MON_HELD = 2,
	MON_ASLEEP = 3, MON_MINDLESS = 4, MON_SILENCED = 5,
	MON_BLINDED = 6, MON_AFRAID = 7, MON_DEAD = 8
};
enum MonsterStatusFlag {
	MONFLAG_AFRAID = 1, MONFLAG_BLIND = 2, MONFLAG_SILENCED = 4,
	MONFLAG_MINDLESS = 8, MONFLAG_ASLEEP = 0x10,
	MONFLAG_HELD = 0x20, MONFLAG_WEBBED = 0x40,
	MONFLAG_PARALYZED = 0x80, MONFLAG_DEAD = 0xff
};

enum MonsterResistUndead {
	MAGIC_RESISTANCE = 0x7f,
	IS_UNDEAD = 0x80
};

enum Fiedl1A {
	FIELD1A_80 = 0x80
};

enum MonsterCounter {
	COUNTER_BITS = 0xf,
	COUNTER_THRESHOLD1 = 0x10, COUNTER_THRESHOLD2 = 0x20,
	COUNTER_REGENERATE = 0x40, COUNTER_ADVANCES = 0x80
};

struct Monster {
	Common::String _name; // char _name[15];
	byte _count;
	byte _fleeThreshold;
	byte _defaultHP;
	byte _defaultAC;
	byte _maxDamage;
	byte _numberOfAttacks;
	byte _speed;
	uint16 _experience;
	byte _field18;
	byte _resistUndead;
	byte _field1a;
	byte _bonusOnTouch;
	byte _specialAbility;
	byte _specialThreshold;
	byte _counterFlags;
	byte _imgNum;

	// Runtime combat fields
	byte _level = 0;
	bool _checked = false;
	byte _status = 0;
	byte _hp = 0;
	byte _ac = 0;

	Common::String getDisplayName() const {
		return _name;
	}
};

class Monsters : public TextParser {
private:
	Monster _monsters[MONSTERS_COUNT];
	Gfx::DTA _monPix;
public:
	Monsters() : _monPix(MONPIX_DTA) {}

	/**
	 * Load the monster list
	 */
	bool load();

	/**
	 * Square brackets operator
	 */
	const Monster &operator[](uint i) {
		assert(i >= 1 && i <= MONSTERS_COUNT);
		return _monsters[i - 1];
	}

	/**
	 * Get a monster image
	 */
	Graphics::ManagedSurface getMonsterImage(int monsterNum);
};

} // namespace MM1
} // namespace MM

#endif
