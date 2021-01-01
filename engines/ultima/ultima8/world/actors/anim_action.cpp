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
#include "ultima/ultima8/world/actors/anim_action.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/kernel/core_app.h"

namespace Ultima {
namespace Ultima8 {

void AnimAction::getAnimRange(unsigned int lastanim, Direction lastdir,
                              bool firststep, Direction dir,
                              unsigned int &startframe, unsigned int &endframe) const {
	startframe = 0;
	endframe = _size;

	if (_flags & AAF_TWOSTEP) {
		const bool looping = hasFlags(AAF_LOOPING);

		// two-step animation?
		if (firststep) {
			if (looping) {
				// for a looping animation, start at the end to
				// make things more fluid
				startframe = _size - 1;
			} else {
				startframe = 0;
			}
			endframe = _size / 2;
		} else {
			// second step starts halfway
			startframe = _size / 2;
			if (looping) {
				endframe = _size - 1;
			}
		}
	} else {
		if (lastanim == _action && lastdir == dir && _size > 1) {
			// skip first frame if repeating an animation
			startframe = 1;
		}
	}
}

void AnimAction::getAnimRange(const Actor *actor, Direction dir,
                              unsigned int &startframe,
                              unsigned int &endframe) const {
	getAnimRange(actor->getLastAnim(), actor->getDir(),
	             actor->hasActorFlags(Actor::ACT_FIRSTSTEP),
	             dir, startframe, endframe);
}

const AnimFrame &AnimAction::getFrame(Direction dir, unsigned int frameno) const {
	uint32 diroff = static_cast<uint32>(dir);
	// HACK for 16 dir support
	if (_dirCount == 8)
		diroff /= 2;

	assert(diroff < _dirCount);
	assert(frameno < _frames[diroff].size());

	return _frames[diroff][frameno];
}

/**
 Translate data file flags of U8 or Crusader into single format, to avoid
 having to check against game type each time they are used
*/
/*static*/
AnimAction::AnimActionFlags AnimAction::loadAnimActionFlags(uint32 rawflags) {
	uint32 ret = AAF_NONE;
	ret |= (rawflags & AAF_COMMONFLAGS);
	if (GAME_IS_U8) {
		if (rawflags & AAF_ATTACK)
			ret |= AAF_ATTACK;
		if (rawflags & AAF_LOOPING2_U8)
			ret |= AAF_LOOPING; // FIXME: handled like this is in pentagram code.. is it used?
	} else {
		assert(GAME_IS_CRUSADER);
		if (rawflags & AAF_ROTATED)
			ret |= AAF_ROTATED;
		if (rawflags & AAF_16DIRS)
			ret |= AAF_16DIRS;
	}

	return static_cast<AnimActionFlags>(ret);
}



} // End of namespace Ultima8
} // End of namespace Ultima
