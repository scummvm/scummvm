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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_PCLASS_R
#define SAGA2_PCLASS_R

namespace Saga2 {

enum {
	protoClassArmor = 1,
	protoClassArrow = 2,
	protoClassBludgeoningWeapon = 4,
	protoClassBookDoc = 5,
	protoClassBottle = 6,
	protoClassBow = 7,
	protoClassEnchantment = 8,
	protoClassEncounterGenerator = 9,
	protoClassFood = 10,
	protoClassIdea = 11,
	protoClassInventory = 13,
	protoClassKey = 14,
	protoClassMap = 16,
	protoClassMemory = 17,
	protoClassMissionGenerator = 19,
	protoClassMonsterGenerator = 20,
	protoClassPhysContainer = 21,
	protoClassPsych = 22,
	protoClassScrollDoc = 24,
	protoClassShield = 25,
	protoClassSkill = 26,
	protoClassSlashingWeapon = 28,
	protoClassTool = 32,
	protoClassWeaponWand = 33,

	// FIXME: Doublecheck these
	protoClassIdeaContainer = 12,
	protoClassMemoryContainer = 18,
	protoClassPsychContainer = 23,
	protoClassSkillContainer = 27
};

} // end of namespace Saga2

#endif
