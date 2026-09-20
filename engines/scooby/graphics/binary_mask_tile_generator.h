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

#ifndef SCOOBY_BINARY_MASK_TILE_GENERATOR_H
#define SCOOBY_BINARY_MASK_TILE_GENERATOR_H

#include "common/scummsys.h"

#include "common/array.h"

#include "scooby/assets/rom.h"

// Reproduces the packed-pixel transformation in Ghidra
// GenerateBinaryMaskTiles at 0x00007A2A without retaining its VRAM
// destination and transfer mechanism.

namespace Scooby {
// Replaces each zero and nonzero source nibble with its caller-selected
// palette index. The inclusive D2 = 0x05EF loop reads 1,520 words from
// 0x00030F12-0x00031AF1 and produces exactly 3,040 bytes.
Common::Array<uint8> generate(const ScoobyDooRom &rom, uint8 nonzeroPaletteIndex,
							  uint8 zeroPaletteIndex);
} // namespace Scooby

#endif // SCOOBY_BINARY_MASK_TILE_GENERATOR_H
