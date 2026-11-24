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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef DUNE_MAP_ANIMATION_H
#define DUNE_MAP_ANIMATION_H

#include "common/scummsys.h"
#include "dune/combat.h"

namespace Dune {

// E3DF: inclusive bounded random using the third DOS seed (D828), not the
// troop/combat streams. Rejected values still advance the seed.
inline uint16 mapAnimationRandom(uint16 &seed, uint16 limit) {
	if (!limit)
		return 0;
	uint16 mask = 1;
	while (mask < limit)
		mask = (mask << 1) | 1;
	uint16 value;
	do {
		value = combatRandom(seed, 0xcbd1) & mask;
	} while (value > limit);
	return value;
}

inline uint mapAnimationLength(const byte *data, uint size, uint list) {
	if (!list || list + 3 >= size)
		return 0;
	// C5CF accepts sprite zero as a STATIC first frame (idle miners).
	// Only subsequent zero entries terminate an animation.
	uint count = 1;
	for (uint p = list + 3; p < size; p += 3) {
		if (!data[p])
			return count;
		if (p + 2 >= size)
			break;
		count++;
	}
	return 0;
}

struct MapUnitAnimation {
	uint16 list = 0, initialList = 0, next = 0, sprite = 0;
	int16 x = 0, y = 0, anchorX = 0, anchorY = 0;
	byte flags = 0;

	void start(const byte *data, uint size, uint16 frames, int atX, int atY, uint16 &seed) {
		*this = MapUnitAnimation();
		const uint count = mapAnimationLength(data, size, frames);
		if (!count)
			return;
		initialList = frames;
		anchorX = atX;
		anchorY = atY;
		// C5CF registers the FIRST sprite at the initial centre. Only the NEXT
		// frame pointer is randomized; no displacement has been applied yet.
		// Its one-frame C60B path never stores an animation-list pointer.
		sprite = data[frames];
		if (count > 1) {
			list = frames;
			next = list + 3 * mapAnimationRandom(seed, count - 1);
			flags = 1;
		}
	}

	void step(const byte *data, uint size) {
		if (!(flags & 1) || !list || next >= size)
			return;
		uint at = next;
		if (!data[at]) {
			if (flags & 2)
				return; // 6B67: a non-looping object holds its last frame.
			at = list;
		}
		if (at + 2 >= size || !data[at])
			return;
		sprite = data[at];
		x = (int16)(x + (int8)data[at + 1]);
		y = (int16)(y + (int8)data[at + 2]);
		next = at + 3;
	}
};

} // End of namespace Dune

#endif
