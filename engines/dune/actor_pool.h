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

#ifndef DUNE_ACTOR_POOL_H
#define DUNE_ACTOR_POOL_H

#include "common/endian.h"
#include "common/rect.h"
#include "common/util.h"

namespace Dune {

enum {
	kSpriteActorCount = 0x3cbe,
	kSpriteActorPool = 0x3cc0,
	kSpriteActorSize = 17,
	kMaxSpriteActors = (0x46d4 - kSpriteActorPool) / kSpriteActorSize
};

inline Common::Rect spriteActorBounds(const byte *actor) {
	return Common::Rect(READ_LE_INT16(actor), READ_LE_INT16(actor + 2),
			READ_LE_INT16(actor + 4), READ_LE_INT16(actor + 6));
}

// 6946 searches live registrations backwards. A selection ring cannot win
// the hit, and the caller decides whether the first hit's owner is eligible.
inline const byte *mapSpriteActorAt(const byte *state, const Common::Point &position) {
	const uint count = MIN<uint>(READ_LE_UINT16(state + kSpriteActorCount), kMaxSpriteActors);
	for (uint i = count; i > 0; --i) {
		const byte *actor = state + kSpriteActorPool + kSpriteActorSize * (i - 1);
		if (actor[12] & 0x40)
			continue;
		if (position.x > READ_LE_INT16(actor) && position.x < READ_LE_INT16(actor + 4) &&
				position.y > READ_LE_INT16(actor + 2) && position.y < READ_LE_INT16(actor + 6))
			return actor;
	}
	return nullptr;
}

// C60B writes only the header. The last four bytes belong to the caller:
// ONMAP animations store next/list, ATTACK uses next as a fraction, and
// static ONMAP actors leave both words untouched in the reused slot.
inline void writeSpriteActor(byte *record, int left, int top, int right, int bottom,
		uint16 sprite, uint16 owner, byte flags) {
	WRITE_LE_UINT16(record, left);
	WRITE_LE_UINT16(record + 2, top);
	WRITE_LE_UINT16(record + 4, right);
	WRITE_LE_UINT16(record + 6, bottom);
	WRITE_LE_UINT16(record + 8, sprite);
	WRITE_LE_UINT16(record + 10, owner);
	record[12] = flags;
}

inline uint appendSpriteActor(byte *state, int left, int top, int right, int bottom,
		uint16 sprite, uint16 owner, byte flags = 0) {
	const uint count = READ_LE_UINT16(state + kSpriteActorCount);
	if (count >= kMaxSpriteActors)
		return 0;
	const uint at = kSpriteActorPool + kSpriteActorSize * count;
	writeSpriteActor(state + at, left, top, right, bottom, sprite, owner, flags);
	WRITE_LE_UINT16(state + kSpriteActorCount, count + 1);
	return at;
}

// C58A marks before painting, then compacts without clearing the last slot.
// The caller handles presentation; both original pool handles relocate here.
inline void removeSpriteActor(byte *state, uint at) {
	const uint count = MIN<uint>(READ_LE_UINT16(state + kSpriteActorCount), kMaxSpriteActors);
	const uint end = kSpriteActorPool + kSpriteActorSize * count;
	if (!count || at < kSpriteActorPool || at >= end || (at - kSpriteActorPool) % kSpriteActorSize)
		return;
	state[at + 12] |= 0x80;
	memmove(state + at, state + at + kSpriteActorSize, end - at - kSpriteActorSize);
	WRITE_LE_UINT16(state + kSpriteActorCount, count - 1);
	for (uint handle = 0x4752; handle <= 0x4754; handle += 2) {
		const uint16 value = READ_LE_UINT16(state + handle);
		if (value >= at)
			WRITE_LE_UINT16(state + handle, value - kSpriteActorSize);
	}
}

} // namespace Dune

#endif
