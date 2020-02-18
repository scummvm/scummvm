/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef WORLD_ACTORS_MONSTERINFO_H
#define WORLD_ACTORS_MONSTERINFO_H

#include "ultima/ultima8/world/actors/treasure_info.h"

namespace Ultima {
namespace Ultima8 {

struct MonsterInfo {
	uint32 _shape;
	uint16 _minHp, _maxHp;
	uint16 _minDex, _maxDex;
	uint16 _minDmg, _maxDmg;
	uint16 _armourClass;
	uint8 _alignment;
	bool _unk;
	uint16 _damageType;
	uint16 _defenseType;
	bool _resurrection; // auto-resurrection after being killed
	bool _ranged; // ranged attack
	bool _shifter; // shapeshifter
	uint32 _explode; // shape to hurl around after being killed (or 0)

	Std::vector<TreasureInfo> _treasure;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
