/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef DUNE_WORM_ANIMATION_H
#define DUNE_WORM_ANIMATION_H

#include "common/array.h"
#include "common/endian.h"

namespace Dune {

struct WormBackStamp {
	uint sprite;
	int x, y;
};

/** 4D6C/4DA0: first VER.BIN animation, poses made of VER.HSQ sprites. */
inline bool wormBackFrame(const byte *data, uint size, uint frame, Common::Array<WormBackStamp> &stamps) {
	stamps.clear();
	if (size < 12)
		return false;
	const uint table = 8 + READ_LE_UINT16(data + 8);
	if (table + 2 > size)
		return false;
	uint at = table + READ_LE_UINT16(data + table);
	Common::Array<uint> frames;
	while (at < size && data[at] != 0xff) {
		frames.push_back(at);
		while (at < size && data[at]) {
			// 4D8B: one introduces a two-byte pose id, all other ids
			// occupy one byte. Zero ends a frame; FF restarts the film.
			if (data[at] == 0xff)
				return false;
			at += data[at] == 1 ? 2 : 1;
		}
		if (at >= size)
			return false;
		at++;
	}
	if (at >= size || frames.empty())
		return false;
	at = frames[frame % frames.size()];
	const int x = READ_LE_INT16(data), y = READ_LE_INT16(data + 2);
	while (data[at]) {
		uint pose = data[at++];
		if (pose == 1)
			pose = 0x100 + data[at++];
		if (pose < 2 || 10 + 2 * (pose - 2) + 2 > size)
			return false;
		uint part = 10 + READ_LE_UINT16(data + 10 + 2 * (pose - 2));
		while (part < size && data[part]) {
			if (part + 3 > size)
				return false;
			const WormBackStamp stamp = { uint(data[part] - 1), x + data[part + 1], y + data[part + 2] };
			stamps.push_back(stamp);
			part += 3;
		}
		if (part >= size)
			return false;
	}
	return true;
}

} // namespace Dune

#endif
