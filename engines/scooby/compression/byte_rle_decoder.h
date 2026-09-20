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

#ifndef SCOOBY_BYTE_RLE_DECODER_H
#define SCOOBY_BYTE_RLE_DECODER_H

#include "common/scummsys.h"

#include "common/array.h"

#include "scooby/assets/rom.h"

// Reproduces Ghidra DecompressByteRle at 0x00009C64, whose signed control
// bytes select literal runs or repeated bytes until the length-prefixed
// output is complete.

namespace Scooby {
// Decodes one ROM stream through the exact output boundary used by Ghidra
// DecompressByteRle at 0x00009C64.
//
// Only the header's low word determines output length. A control byte with
// bit 7 clear copies (control + 1) literals; one with bit 7 set repeats the
// next byte (257 - control) times.
Common::Array<uint8> decompressByteRle(const ScoobyDooRom &rom, int sourceOffset);
} // namespace Scooby

#endif // SCOOBY_BYTE_RLE_DECODER_H
