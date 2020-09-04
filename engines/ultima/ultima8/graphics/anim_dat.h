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

#ifndef ULTIMA8_GRAPHICS_ANIMDAT_H
#define ULTIMA8_GRAPHICS_ANIMDAT_H

#include "ultima/shared/std/containers.h"
#include "ultima/ultima8/world/actors/animation.h"

namespace Ultima {
namespace Ultima8 {

class AnimAction;
class ActorAnim;
class Actor;

class AnimDat {
public:
	AnimDat();
	~AnimDat();

	void load(Common::SeekableReadStream *rs);

	const ActorAnim *getAnim(uint32 shape) const;
	const AnimAction *getAnim(uint32 shape, uint32 action) const;

	//! Return the action number for a given animation sequence on the given actor
	static uint32 getActionNumberForSequence(Animation::Sequence action, const Actor *actor);
private:
	Std::vector<ActorAnim *> _anims;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
