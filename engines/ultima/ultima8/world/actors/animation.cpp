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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/world/actors/animation.h"
#include "ultima/ultima8/kernel/core_app.h"

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

bool isCombatAnimCru(const Sequence anim) {
	switch (anim) {
	case combatStand:
	case readyWeapon:
	case advance:
	case retreat:
	case attack:
	case kick:
	case kneel:
	case kneelStart:
	case fire2:
	case combatRollLeft:
	case combatRollRight:
	case slideLeft:
	case slideRight:
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
