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

#include "ultima/ultima8/world/actors/animation.h"
#include "ultima/ultima8/ultima8.h"

namespace Ultima {
namespace Ultima8 {
namespace Animation {

bool isCombatAnim(const Sequence anim) {
	if (GAME_IS_U8)
		return isCombatAnimU8(anim);
	else
		return isCombatAnimCru(anim);
}

bool isCombatAnimU8(const Sequence anim) {
	switch (anim) {
	case combatStand:
	case readyWeapon:
	case advance:
	case retreat:
	case attack:
	case kick:
	case startBlock:
	case stopBlock:
		return true;
	default:
		return false;
	}
}

bool isCastAnimU8(const Sequence anim) {
	switch (anim) {
	case cast1:
	case cast2:
	case cast3:
	case cast4:
	case cast5:
		return true;
	default:
		return false;
	}
}

bool isCombatAnimCru(const Sequence anim) {
	switch (anim & ~crusaderAbsoluteAnimFlag) {
	case combatStand:
	case combatStandSmallWeapon:
	case combatStandLargeWeapon:
	case readyWeapon:
	case advance:
	case retreat:
	case attack:
	case reloadSmallWeapon:
	case kick:
	case kneel:
	case kneelStartCru:
	case kneelEndCru:
	case kneelAndFire:
	case brightFireLargeWpn:
	case kneelCombatRollLeft:
	case kneelCombatRollRight:
	case combatRollLeft:
	case combatRollRight:
	case slideLeft:
	case slideRight:
	case startRun:
	case run:
	case stopRunningAndDrawSmallWeapon:
	case kneelingAdvance:
	case kneelingRetreat:
		return true;
	default:
		return false;
	}
}

/** determines if we need to ready or unready our weapon */
Sequence checkWeapon(const Sequence nextanim,
					 const Sequence lastanim) {
	Sequence anim = nextanim;
	if (isCombatAnim(nextanim) && !isCombatAnim(lastanim)) {
		anim = readyWeapon;
	} else if (!isCombatAnim(nextanim) && isCombatAnim(lastanim)) {
		anim = unreadyWeapon;
	}
	return anim;
}

} // End of namespace Animation
} // End of namespace Ultima8
} // End of namespace Ultima
