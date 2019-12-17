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

#ifndef WORLD_ACTORS_ANIMATION_H
#define WORLD_ACTORS_ANIMATION_H

namespace Ultima {
namespace Ultima8 {
namespace Animation {

enum Sequence {
	walk = 0,
	run = 1,
	stand = 2,
	jumpUp = 3,
	standUp = 4,
	readyWeapon = 5,
	unreadyWeapon = 6,
	attack = 7,
	advance = 8,
	retreat = 9,
	runningJump = 10,
	shakeHead = 11,
	step = 12,
	stumbleBackwards = 13,
	die = 14,
	combatStand = 15,
	land = 16,
	jump = 17,
	airwalkJump = 18,
	//19-26: climbing up on increasingly high objects
	climb16 = 19,
	climb24 = 20,
	climb32 = 21,
	climb40 = 22,
	climb48 = 23,
	climb56 = 24,
	climb64 = 25,
	climb72 = 26,
	//27-31: casting magic
	cast1 = 27,
	cast2 = 28,
	cast3 = 29,
	cast4 = 30,
	cast5 = 31,
	lookLeft = 32,
	lookRight = 33,
	startKneeling = 34,
	kneel = 35,
	//36: Vividos only: magic?
	//37: Mythran only: magic?
	//38: Vividos only: ?
	//39: unused
	//40: ? - could be a slow attack or quick block ???
	//41: unused
	keepBalance = 42,
	//43: unused
	fallBackwards = 44,
	hang = 45,
	climbUp = 46,
	idle1 = 47,
	idle2 = 48,
	kneel2 = 49,
	stopKneeling = 50,
	sitDownInChair = 51,
	standUpFromChair = 52,
	talk = 53,
	//54: Mythran and Vividos only: magic?
	work = 55,
	drown = 56,
	burn = 57,
	kick = 58,
	startBlock = 59,
	stopBlock = 60
	            //61: unused
	            //62: unused
	            //63: unused
};

enum Result {
	FAILURE = 0,
	SUCCESS = 1,
	END_OFF_LAND = 2
};

bool isCombatAnim(const Sequence anim);
Sequence checkWeapon(const Sequence nextanim, const Sequence lastanim);

} // End of namespace Animation
} // End of namespace Ultima8
} // End of namespace Ultima

#endif
