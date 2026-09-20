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

#ifndef SCOOBY_GENESIS_SPRITE_TABLE_DECODER_H
#define SCOOBY_GENESIS_SPRITE_TABLE_DECODER_H

#include "common/array.h"

#include "scooby/common/span.h"
#include "sprite_instance.h"

// Decodes the linked sprite attributes transferred by Ghidra RunMainMenu at
// 0x00008196 into backend-neutral scene objects.
//
// Original VDP register 5 value 0x7E selects sprite table 0xFC00. The menu
// asset at 0x000085F8-0x0000865F contains twelve visible linked entries and
// one off-screen terminator. Decoding retains coordinates, shape, palette,
// whole-sprite flips, priority, link order, and Genesis vertical-column tile
// order; the VRAM base and link indices are discarded after ordering.

namespace Scooby {
// Follows the authored link order from sprite zero through the terminal
// zero link. Returns sprites in highest-to-lowest linked priority order.
Common::Array<SpriteInstance> decode(Span<const uint8> packedEntries);
} // namespace Scooby

#endif // SCOOBY_GENESIS_SPRITE_TABLE_DECODER_H
