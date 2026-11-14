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

#ifndef SCOOBY_ACTOR_SPRITE_TRANSFORMER_H
#define SCOOBY_ACTOR_SPRITE_TRANSFORMER_H

#include "common/scummsys.h"

#include "common/array.h"

#include "scooby/assets/rom.h"

// Owns the recovered compact actor-pixel layouts, selection masks, and scale conversions.

namespace Scooby {

class ActorSpriteTransformer {
public:
	// Copies the executable scale-mask table from the verified cartridge into converter-owned state.
	// rom: verified cartridge containing the original 256 selection records.
	explicit ActorSpriteTransformer(const ScoobyDooRom &rom);

	// Packs one unscaled 48x80 compact actor frame into the primary sixty-tile layout.
	// decodedPixels: complete byte-RLE output containing one byte per source pixel.
	// sourceOffset: first source byte corresponding to the original fixed workspace window.
	// Returns exactly sixty packed Genesis tiles in the primary actor publication order.
	//
	// Ghidra: expandCompactSprite48UsingPrimaryLayout (0x0000D3F0). Original
	// g_abCompactSpritePixelWorkspace at RAM 0xFF6000-0xFF6EFF has no consumed image-backed value:
	// UpdateActorAnimationFrames decodes all 3,840 source bytes before this call. The 480-word permutation at
	// 0x0000BC70-0x0000C02F covers every four-byte output offset through 0x077C exactly once.
	Common::Array<uint8> expandCompactSprite48UsingPrimaryLayout(
		const Common::Array<uint8> &decodedPixels,
		int sourceOffset) const;

	// Packs one unscaled 80x48 compact actor frame into the secondary sixty-tile layout.
	// decodedPixels: complete byte-RLE output containing one byte per source pixel.
	// sourceOffset: first source byte corresponding to the original fixed workspace window.
	// Returns exactly sixty packed Genesis tiles in the secondary actor publication order.
	//
	// Ghidra: expandCompactSprite80UsingSecondaryLayout (0x0000D42C). Original
	// g_abCompactSpritePixelWorkspace at RAM 0xFF6000-0xFF6EFF has no consumed image-backed value. The
	// 480-word permutation at 0x0000C030-0x0000C3EF covers every four-byte output offset through 0x077C
	// exactly once.
	Common::Array<uint8> expandCompactSprite80UsingSecondaryLayout(
		const Common::Array<uint8> &decodedPixels, int sourceOffset) const;

	// Scales a 48x80 one-byte-per-pixel compact actor frame and packs the retained pixels into the original
	// sixty-tile publication order.
	// decodedPixels: decoded compact pixels; each retained byte becomes one packed 4-bpp pixel.
	// sourceOffset: first byte of the 48x80 source within the decoded stream.
	// horizontalScale: selection-record index controlling retained source columns.
	// verticalScale: selection-record index controlling retained source rows.
	// Returns exactly sixty packed Genesis tiles, including explicit zero padding after the scaled extent.
	//
	// Ghidra: scaleCompactSprite48 (0x0000D468). The 480-word permutation at 0x0000BC70-0x0000C02F maps rows
	// 0-31 as row*4 + longword*0x80, rows 32-63 from base 0x300 with the same stride, and rows 64-79 from base
	// 0x600 with longword stride 0x40; every call replaces or clears all 1,920 output bytes.
	Common::Array<uint8> scaleCompactSprite48(const Common::Array<uint8> &decodedPixels,
											  int sourceOffset,
											  uint16 horizontalScale,
											  uint16 verticalScale) const;

	// Scales an 80x48 one-byte-per-pixel compact actor frame and packs the retained pixels into the original
	// sixty-tile publication order.
	// decodedPixels: decoded compact pixels; each retained byte becomes one packed 4-bpp pixel.
	// sourceOffset: first byte of the 80x48 source within the decoded stream.
	// horizontalScale: selection-record index controlling retained source columns.
	// verticalScale: selection-record index controlling retained source rows.
	// Returns exactly sixty packed Genesis tiles, including explicit zero padding after the scaled extent.
	//
	// Ghidra: scaleCompactSprite80 (0x0000DA6A). The 480-word permutation at 0x0000C030-0x0000C3EF maps rows
	// 0-31 as row*4 + longword*0x80 and rows 32-47 from base 0x500 with longword stride 0x40; every call
	// replaces or clears all 1,920 output bytes.
	Common::Array<uint8> scaleCompactSprite80(const Common::Array<uint8> &decodedPixels,
											  int sourceOffset,
											  uint16 horizontalScale,
											  uint16 verticalScale) const;

private:
	static const int kScaleMaskTableOffset = 0x0000C3F0;
	static const int kScaleMaskRecordSize = 16;
	static const int kScaleMaskRecordCount = 256;
	static const int kSourceWidth48 = 48;
	static const int kSourceHeight48 = 80;
	static const int kOutputLongwordsPerRow48 = kSourceWidth48 / 8;
	static const int kOutputBytes48 = kSourceWidth48 * kSourceHeight48 / 2;
	static const int kSourceWidth80 = 80;
	static const int kSourceHeight80 = 48;
	static const int kOutputLongwordsPerRow80 = kSourceWidth80 / 8;
	static const int kOutputBytes80 = kSourceWidth80 * kSourceHeight80 / 2;

	bool maskSelects(uint16 scale, int sourceIndex) const;

	// Immutable copy of Ghidra g_aubScaleSelectionMasks at ROM 0x0000C3F0-0x0000D3EF. The image starts with a
	// sixteen-byte all-selected record and stores 256 sixteen-byte fixed-point selection records.
	Common::Array<uint8> _scaleSelectionMasks;
};
} // namespace Scooby

#endif // SCOOBY_ACTOR_SPRITE_TRANSFORMER_H
