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

#ifndef WORLD_ACTORS_WEAPONOVERLAY_H
#define WORLD_ACTORS_WEAPONOVERLAY_H

#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima8 {

struct WeaponOverlayFrame {
	int32 _xOff;
	int32 _yOff;
	uint32 _frame;
};

struct WeaponOverlay {
	unsigned int _dirCount;
	Std::vector<WeaponOverlayFrame> *_frames; // 8 or 16 directions

	WeaponOverlay() : _frames(nullptr), _dirCount(0) {
	}
	~WeaponOverlay() {
		delete[] _frames;
	}
};

struct AnimWeaponOverlay {
	//! get the weapon overlay info for a specific animation frame
	//! \param type the overlay type
	//! \param direction the direction
	//! \param frame the animation frame
	//! \return nullptr if invalid, or pointer to a frame; don't delete it.
	const WeaponOverlayFrame *getFrame(unsigned int type,
	                                   Direction direction,
	                                   unsigned int frame) const {
		if (type >= _overlay.size())
			return nullptr;

		assert(direction != dir_invalid);

		uint32 diroff;
		if (_overlay[type]._dirCount == 8)
			diroff = static_cast<uint32>(direction) / 2;
		else
			diroff = static_cast<uint32>(direction);

		if (diroff >= _overlay[type]._dirCount)
			return nullptr;
		if (frame >= _overlay[type]._frames[diroff].size())
			return nullptr;
		return &(_overlay[type]._frames[diroff][frame]);
	}

	Std::vector<WeaponOverlay> _overlay;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
