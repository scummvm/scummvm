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

#ifndef ULTIMA4_CORE_TYPES_H
#define ULTIMA4_CORE_TYPES_H

#include "ultima/ultima4/map/direction.h"
#include "common/scummsys.h"

namespace Ultima {
namespace Ultima4 {

class Tile;

typedef uint TileId;
typedef byte MapId;

enum TileSpeed {
	FAST,
	SLOW,
	VSLOW,
	VVSLOW
};

enum TileEffect {
	EFFECT_NONE,
	EFFECT_FIRE,
	EFFECT_SLEEP,
	EFFECT_POISON,
	EFFECT_POISONFIELD,
	EFFECT_ELECTRICITY,
	EFFECT_LAVA
};

enum TileAnimationStyle {
	ANIM_NONE,
	ANIM_SCROLL,
	ANIM_CAMPFIRE,
	ANIM_CITYFLAG,
	ANIM_CASTLEFLAG,
	ANIM_SHIPFLAG,
	ANIM_LCBFLAG,
	ANIM_FRAMES
};

/**
 * An Uncopyable has no default copy constructor of operator=.  A subclass may derive from
 * Uncopyable at any level of visibility, even private, and subclasses will not have a default copy
 * constructor or operator=. See also, boost::noncopyable Uncopyable (from the Boost project) and
 * Item 6 from Scott Meyers Effective C++.
 */
class Uncopyable {
protected:
	Uncopyable() {}
	~Uncopyable() {}

private:
	Uncopyable(const Uncopyable &);
	const Uncopyable &operator=(const Uncopyable &);
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
