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

#include "ultima8/misc/pent_include.h"
#include "ultima8/world/actors/AnimAction.h"
#include "ultima8/world/actors/actor.h"

namespace Ultima8 {

void AnimAction::getAnimRange(unsigned int lastanim, int lastdir,
                              bool firststep, int dir,
                              unsigned int &startframe, unsigned int &endframe) const {
	startframe = 0;
	endframe = size;

	if (flags & AAF_TWOSTEP) {
		// two-step animation?
		if (firststep) {
			if (flags & (AAF_LOOPING | AAF_LOOPING2)) {// CHECKME: unknown flag
				// for a looping animation, start at the end to
				// make things more fluid
				startframe = size - 1;
			} else {
				startframe = 0;
			}
			endframe = size / 2;
		} else {
			// second step starts halfway
			startframe = size / 2;
			if (flags & (AAF_LOOPING | AAF_LOOPING2)) {// CHECKME: unknown flag
				endframe = size - 1;
			}
		}
	} else {
		if (lastanim == action && lastdir == dir && size > 1) {
			// skip first frame if repeating an animation
			startframe = 1;
		}
	}
}

void AnimAction::getAnimRange(Actor *actor, int dir,
                              unsigned int &startframe,
                              unsigned int &endframe) const {
	getAnimRange(actor->getLastAnim(), actor->getDir(),
	             (actor->getActorFlags() & Actor::ACT_FIRSTSTEP) != 0,
	             dir, startframe, endframe);
}

} // End of namespace Ultima8
