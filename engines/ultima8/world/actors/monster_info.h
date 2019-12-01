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

#include "ultima8/world/actors/treasure_info.h"

namespace Ultima8 {

struct MonsterInfo {
	uint32 shape;
	uint16 min_hp, max_hp;
	uint16 min_dex, max_dex;
	uint16 min_dmg, max_dmg;
	uint16 armour_class;
	uint8 alignment;
	bool unk;
	uint16 damage_type;
	uint16 defense_type;
	bool resurrection; // auto-resurrection after being killed
	bool ranged; // ranged attack
	bool shifter; // shapeshifter
	uint32 explode; // shape to hurl around after being killed (or 0)

	std::vector<TreasureInfo> treasure;
};

} // End of namespace Ultima8

#endif
