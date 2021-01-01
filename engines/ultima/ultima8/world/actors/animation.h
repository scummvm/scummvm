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
	//39: unused in u8
	//40: ? - could be a slow attack or quick block ???
	//41: unused in u8
	keepBalance = 42,
	//43: unused in u8
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
	stopBlock = 60,
	            //61: unused in u8
	            //62: unused in u8
	            //63: unused in u8
	// Some crusader-specific animations (some use the same IDs as above)
	reload = 15,
	combatRollLeft = 23,
	combatRollRight = 24,
	walkWithGun = 25,
	kneelAndFire = 26,
	slideLeft = 28,
	slideRight = 29,
	unknownAnim30 = 30,
	startRunWithLargeWeapon = 31,
	teleportIn = 32,
	teleportOut = 33,
	startRunWithSmallWeapon = 34,
	startRunWithLargeWeapon2 = 35,
	advanceWithSmallWeapon = 36,
	halfStep = 37,
	startRun = 38,
	stopRunningAndDrawWeapon = 39,
	kneelStart = 40,
	kneelEnd = 41,
	kneelAndFireSmallWeapon = 42,
	kneelAndFireLargeWeapon = 43,
	advanceWithLargeWeapon = 44,
	quickRetreat = 45,
	kneelingWithSmallWeapon = 46,
	kneelingWithLargeWeapon = 47,
	combatRun = 48,
	runWithLargeWeapon = 49,
	runWithLargeWeapon2 = 50,
	kneelingRetreat = 51,
	kneelingAdvance = 52,
	kneelingSlowRetreat = 53,
	fire2 = 54,
	electrocuted = 55,
	jumpForward = 56,
	surrender = 57,
	anotherJump = 58,
	jumpLanding = 59,
	surrenderStand = 60,
	slowCombatRollLeft = 61,
	slowCombatRollRight = 62,
	finishFiring = 63,

	crusaderAbsoluteAnimFlag = 0x1000, //!< Bit mask magic to say we want an exact number, don't do mapping from U8 animation numbers
	teleportInReplacement = crusaderAbsoluteAnimFlag | teleportIn,	//!< See notes in Actor::receiveHitCru
	teleportOutReplacement = crusaderAbsoluteAnimFlag | teleportOut	//!< See notes in Actor::receiveHitCru
};

enum Result {
	FAILURE = 0,
	SUCCESS = 1,
	END_OFF_LAND = 2
};

bool isCombatAnim(const Sequence anim);
bool isCombatAnimU8(const Sequence anim);
bool isCombatAnimCru(const Sequence anim);
Sequence checkWeapon(const Sequence nextanim, const Sequence lastanim);

} // End of namespace Animation
} // End of namespace Ultima8
} // End of namespace Ultima

#endif
