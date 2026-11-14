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

#ifndef SCOOBY_RING_LZ_DECODER_H
#define SCOOBY_RING_LZ_DECODER_H

#include "common/scummsys.h"

#include "common/array.h"

#include "scooby/assets/rom.h"

// Reproduces Ghidra DecompressRingLz at 0x000097AA, the ROM's MSB-first
// literal/back-reference stream decoder.

namespace Scooby {
// Reimplements Ghidra DecompressRingLz at 0x000097AA through its reserved
// zero back-reference terminator.
//
// One-bit tags are consumed MSB first. Literals carry eight bits; matches
// carry a 12-bit absolute offset and a four-bit length encoding 2-17 bytes.
// The zero-filled 4 KiB ring starts writing at index one, and match offset
// zero terminates the stream.
Common::Array<uint8> decompressRingLz(const ScoobyDooRom &rom, int sourceOffset);
} // namespace Scooby

#endif // SCOOBY_RING_LZ_DECODER_H
