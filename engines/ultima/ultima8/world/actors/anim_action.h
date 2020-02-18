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

#ifndef WORLD_ACTORS_ANIMACTION_H
#define WORLD_ACTORS_ANIMACTION_H

#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima8 {

class Actor;

struct AnimFrame {
	int _frame;
	int _deltaZ;
	int _deltaDir;
	int _sfx;
	uint32 _flags;

	enum AnimFrameFlags {
		AFF_UNK1     = 0x0001,
		AFF_ONGROUND = 0x0002,
		AFF_FLIPPED  = 0x0020,
		AFF_SPECIAL  = 0x0800
	};

	inline bool is_flipped() {
		return (_flags & AFF_FLIPPED) != 0;
	}
	inline int attack_range() {
		return ((_flags >> 2) & 0x07);
	}
};

struct AnimAction {
	uint32 _shapeNum;
	uint32 _action;

	Std::vector<AnimFrame> frames[16]; // 8 or 16 directions
	unsigned int _size;
	int _frameRepeat;
	uint32 _flags;

	unsigned int _dirCount;

	//! return the range of the animation to play
	//! \param actor The actor to play the animation for
	//! \param dir The direction
	//! \param startframe The first frame to play
	//! \param endframe The frame after the last frame to play
	void getAnimRange(Actor *actor, int dir,
	                  unsigned int &startframe, unsigned int &endframe) const;

	//! return the range of the animation to play
	//! \param lastanim The lastanim of the Actor
	//! \param lastdir The direction of the Actor
	//! \param firststep The firststep flag of the Actor
	//! \param dir The direction
	//! \param startframe The first frame to play
	//! \param endframe The frame after the last frame to play
	void getAnimRange(unsigned int lastanim, int lastdir,
	                  bool firststep, int dir,
	                  unsigned int &startframe, unsigned int &endframe) const;

	unsigned int getDirCount() const;

	enum AnimActionFlags {
		AAF_TWOSTEP      = 0x0001,
		AAF_ATTACK       = 0x0002,
		AAF_LOOPING      = 0x0004,
		AAF_UNSTOPPABLE  = 0x0008,
		AAF_LOOPING2     = 0x0010, // CHECKME: guessing at this flag
		AAF_HANGING      = 0x0080,
		AAF_CRUS_16DIRS  = 0x4000, // Crusader
		AAF_DESTROYACTOR = 0x8000  // destroy actor after animation finishes
	};
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
