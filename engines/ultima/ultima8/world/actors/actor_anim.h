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

#ifndef WORLD_ACTORS_ACTORANIM_H
#define WORLD_ACTORS_ACTORANIM_H

#include "ultima/shared/std/containers.h"
#include "ultima/ultima8/world/actors/anim_action.h"

namespace Ultima {
namespace Ultima8 {

class ActorAnim {
	friend class AnimDat;
public:
	ActorAnim() {}
	~ActorAnim() {
		for (unsigned int i = 0; i < _actions.size(); ++i)
			delete _actions[i];
	}

	AnimAction *getAction(unsigned int n) {
		if (n >= _actions.size())
			return nullptr;
		return _actions[n];
	}

private:
	Std::vector<AnimAction *> _actions; // list of this actor's actions
	// (0 if actor doesn't have action)
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
