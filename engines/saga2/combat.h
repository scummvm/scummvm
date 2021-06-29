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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_COMBAT_H
#define SAGA2_COMBAT_H

namespace Saga2 {

class Location;

enum weaponHitType {
	hitMiss     = 0,
	hitShield,
	hitTarget
};

//-----------------------------------------------------------------------

#define nullWeapon 0

enum combatSound {
	arrowFlesh  = 1,
	arrowWood,
	bluntBreak,
	bluntFlesh,
	bluntMetal,
	swordBreak,
	swordFlesh,
	swordMetal,
	whoosh1,
	whoosh2,
	whoosh3
};


void makeCombatSound(uint8 cs, Location l);

void makeGruntSound(uint8 cs, Location l);

} // end of namespace Saga2

#endif
