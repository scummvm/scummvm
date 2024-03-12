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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef M4_PLATFORM_DRAW_H
#define M4_PLATFORM_DRAW_H

#include "m4/m4_types.h"

namespace M4 {

/**
 * Decode an 8BPP RLE encoded image
 */
void RLE8Decode(const uint8 *inBuff, uint8 *outBuff, uint32 pitch);

/**
 * Given RLE data, skips over a given number of encoded lines
 */
uint8 *SkipRLE_Lines(uint32 linesToSkip, uint8 *rleData);

/**
 * Returns the size of an RLE encoded image
 * @param data		RLE data
 * @param pitch		Image pitch
 * @returns			Decoded image's size in bytes
 */
size_t RLE8Decode_Size(byte *src, int pitch);

} // namespace M4

#endif
