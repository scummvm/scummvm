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

#ifndef ULTIMA8_WORLD_ARMOURINFO_H
#define ULTIMA8_WORLD_ARMOURINFO_H

namespace Ultima {
namespace Ultima8 {

struct ArmourInfo {
	uint32 _shape;
	uint32 _frame;
	uint16 _armourClass;
	uint16 _kickAttackBonus;
	uint16 _defenseType; // see WeaponInfo struct
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
