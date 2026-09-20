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

#ifndef SCOOBY_PASSWORD_ENTRY_DECODER_H
#define SCOOBY_PASSWORD_ENTRY_DECODER_H

#include "common/scummsys.h"

#include "scooby/common/span.h"

// Owns the recovered inverse transforms used only while validating entered password symbols.

namespace Scooby {
// Decodes the room and progress bytes from their prefix-XOR representation in place.
// packedPassword: prefix, checksum, and 29 chained state bytes.
//
// Ghidra: DecodePasswordSymbolXorChain (0x0000876C). The complete contiguous body is
// 0x0000876C-0x0000879D. The span replaces named shared workspace
// g_bSharedPasswordRoomOrSpriteScratch001E at 0xFF001E,
// g_bSharedPasswordChecksumOrSpriteScratch001F at 0xFF001F, and
// g_abSharedPasswordStateOrSpriteScratch0020 at 0xFF0020-0xFF003C; its caller replaces
// all meaningful bytes before this function runs.
void DecodePasswordSymbolXorChain(Span<uint8> packedPassword);

// Assembles the next eight password-stream bits into one byte.
// symbols: the 50 entered alphabet indices.
// bitOffset: zero-based stream position advanced by eight.
// Returns the next most-significant-first packed byte.
//
// Ghidra: ReadPasswordByteFromSymbols (0x00009382). The complete contiguous body is
// 0x00009382-0x00009391. The byte return replaces the low eight bits of native D2 while
// bitOffset preserves the A0, D1, and D3 cursor state across calls.
uint8 ReadPasswordByteFromSymbols(Span<const uint8> symbols, int &bitOffset);
} // namespace Scooby

#endif // SCOOBY_PASSWORD_ENTRY_DECODER_H
