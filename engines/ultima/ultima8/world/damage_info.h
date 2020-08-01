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

#ifndef WORLD_DAMAGE_INFO_H
#define WORLD_DAMAGE_INFO_H

#include "common/stream.h"

#include "ultima/shared/std/string.h"

namespace Ultima {
namespace Ultima8 {

class Item;

/**
 * The damage.dat flex contains data about each shape and how it should be damaged
 */
class DamageInfo {
public:
	DamageInfo(uint8 data[6]);
	~DamageInfo() {};

	//! apply this damage info to the given item.  Returns true if the item was destroyed in the process.
	bool applyToItem(Item *item, uint16 points) const;

	bool frameDataIsAbsolute() const {
		return (_flags >> 7) & 1;
	}
	bool replaceItem() const {
		return (_flags >> 6) & 1;
	}
	bool explodeDestroysItem() const {
		return (_flags >> 5) & 1;
	}
	bool explodeWithDamage() const {
		return (_flags >> 3) & 1;
	}
	bool takesDamage() const {
		return _flags & 1;
	}

	uint8 damagePoints() const {
		return _damagePoints;
	}

protected:
	bool explode() const {
		return (_flags & 0x06) != 0;
	}
	int explosionType() const {
		assert(explode());
		return ((_flags & 0x06) >> 1) - 1;
	}

	uint16 getReplacementShape() const {
		assert(replaceItem());
		return static_cast<uint16>(_data[1]) << 8 | _data[0];
	}

	uint16 getReplacementFrame() const {
		assert(replaceItem());
		return static_cast<uint16>(_data[2]);
	}


	// Flags are ABCxDEEF
	// A = frame data is absolute (not relative to current)
	// B = item is replaced when destroyed
	// C = item destroyed after explosion
	// D = explosion damages surrounding items
	// EE = 2 bits for explosion type
	// F = item takes damage
	uint8 _flags;
	uint8 _sound;
	uint8 _data[3];
	uint8 _damagePoints;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
