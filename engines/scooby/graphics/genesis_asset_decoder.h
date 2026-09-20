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

#ifndef SCOOBY_GENESIS_ASSET_DECODER_H
#define SCOOBY_GENESIS_ASSET_DECODER_H

#include "common/scummsys.h"

#include "common/array.h"

#include "palette_color.h"
#include "scooby/assets/rom.h"
#include "scooby/common/span.h"
#include "tile_cell.h"

// Converts the original ROM's packed graphics formats into high-level scene
// assets at load time. Console memory and transfer behavior do not cross
// this boundary.

namespace Scooby {
// Expands packed 4-bpp ROM tiles into one palette index per pixel. Returns
// decoded 8x8 tiles in row-major order.
Common::Array<uint8> decodeTiles(Span<const uint8> packedTiles);

// Decodes one big-endian tilemap word into scene metadata.
TileCell decodeTileCell(Span<const uint8> packedCells, int byteOffset);

// Decodes one packed tilemap word that has already been read into host byte order.
TileCell decodeTileCell(uint16 entry);

// Encodes one logical tile cell into its original packed name-table word.
uint16 encodeTileCell(const TileCell &cell);

// Reads and decodes a contiguous palette range beginning at a recovered ROM address.
Common::Array<PaletteColor> readPalette(const ScoobyDooRom &rom, int romOffset, int colorCount = 64);
} // namespace Scooby

#endif // SCOOBY_GENESIS_ASSET_DECODER_H
