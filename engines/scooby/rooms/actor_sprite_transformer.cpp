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

#include "actor_sprite_transformer.h"

#include "common/array.h"

namespace Scooby {
namespace {
inline void WriteUInt16BigEndian(Common::Array<uint8> &output, int offset, uint16 value) {
	output[static_cast<std::size_t>(offset)] = static_cast<uint8>(value >> 8);
	output[static_cast<std::size_t>(offset + 1)] = static_cast<uint8>(value);
}

inline void WriteUInt32BigEndian(Common::Array<uint8> &output, int offset, uint32 value) {
	output[static_cast<std::size_t>(offset)] = static_cast<uint8>(value >> 24);
	output[static_cast<std::size_t>(offset + 1)] = static_cast<uint8>(value >> 16);
	output[static_cast<std::size_t>(offset + 2)] = static_cast<uint8>(value >> 8);
	output[static_cast<std::size_t>(offset + 3)] = static_cast<uint8>(value);
}
} // namespace

ActorSpriteTransformer::ActorSpriteTransformer(const ScoobyDooRom &rom)
	: _scaleSelectionMasks(
		  rom.readBytes(kScaleMaskTableOffset, kScaleMaskRecordCount * kScaleMaskRecordSize).toArray()) {
}

Common::Array<uint8> ActorSpriteTransformer::expandCompactSprite48UsingPrimaryLayout(
	const Common::Array<uint8> &decodedPixels, int sourceOffset) const {
	// Ghidra 0x0000D3F0-0x0000D3FF: select the primary 480-word output permutation, the decoded pixel
	// workspace, and the terminal-inclusive 0x01DF DBF count.
	Common::Array<uint8> output(static_cast<std::size_t>(kOutputBytes48), 0);
	for (int sourceRow = 0; sourceRow < kSourceHeight48; ++sourceRow) {
		for (int longword = 0; longword < kOutputLongwordsPerRow48; ++longword) {
			// Ghidra 0x0000D400-0x0000D425: shift and OR eight source bytes into two independent 16-bit
			// words, then publish them at the next primary-layout permutation offset.
			int sourceLongwordOffset = sourceOffset + (sourceRow * kOutputLongwordsPerRow48 + longword) * 8;
			uint16 packedHighWord = 0;
			uint16 packedLowWord = 0;
			for (int pixel = 0; pixel < 4; ++pixel) {
				packedHighWord <<= 4;
				packedHighWord |= decodedPixels[static_cast<std::size_t>(sourceLongwordOffset + pixel)];
				packedLowWord <<= 4;
				packedLowWord |= decodedPixels[static_cast<std::size_t>(sourceLongwordOffset + 4 + pixel)];
			}

			int destinationOffset = sourceRow < 32
										? sourceRow * 4 + longword * 0x80
									: sourceRow < 64
										? 0x300 + (sourceRow - 32) * 4 + longword * 0x80
										: 0x600 + (sourceRow - 64) * 4 + longword * 0x40;
			WriteUInt16BigEndian(output, destinationOffset, packedHighWord);
			WriteUInt16BigEndian(output, destinationOffset + 2, packedLowWord);
		}
	}

	// Ghidra 0x0000D426-0x0000D42B: DBF visits all 480 offsets before returning the complete buffer.
	return output;
}

Common::Array<uint8> ActorSpriteTransformer::expandCompactSprite80UsingSecondaryLayout(
	const Common::Array<uint8> &decodedPixels, int sourceOffset) const {
	// Ghidra 0x0000D42C-0x0000D43B: select the secondary 480-word output permutation, the decoded pixel
	// workspace, and the terminal-inclusive 0x01DF DBF count.
	Common::Array<uint8> output(static_cast<std::size_t>(kOutputBytes80), 0);
	for (int sourceRow = 0; sourceRow < kSourceHeight80; ++sourceRow) {
		for (int longword = 0; longword < kOutputLongwordsPerRow80; ++longword) {
			// Ghidra 0x0000D43C-0x0000D465: shift and OR eight source bytes into two independent 16-bit
			// words, then publish them at the next secondary-layout permutation offset.
			int sourceLongwordOffset = sourceOffset + (sourceRow * kOutputLongwordsPerRow80 + longword) * 8;
			uint16 packedHighWord = 0;
			uint16 packedLowWord = 0;
			for (int pixel = 0; pixel < 4; ++pixel) {
				packedHighWord <<= 4;
				packedHighWord |= decodedPixels[static_cast<std::size_t>(sourceLongwordOffset + pixel)];
				packedLowWord <<= 4;
				packedLowWord |= decodedPixels[static_cast<std::size_t>(sourceLongwordOffset + 4 + pixel)];
			}

			int destinationOffset =
				sourceRow < 32
					? sourceRow * 4 + longword * 0x80
					: 0x500 + (sourceRow - 32) * 4 + longword * 0x40;
			WriteUInt16BigEndian(output, destinationOffset, packedHighWord);
			WriteUInt16BigEndian(output, destinationOffset + 2, packedLowWord);
		}
	}

	// Ghidra 0x0000D466-0x0000D467: return the complete sixty-tile buffer.
	return output;
}

Common::Array<uint8> ActorSpriteTransformer::scaleCompactSprite48(
	const Common::Array<uint8> &decodedPixels,
	int sourceOffset,
	uint16 horizontalScale,
	uint16 verticalScale) const {
	// Ghidra 0x0000D468-0x0000D499: select the vertical and horizontal fixed-point mask records and
	// initialize the 80-row source traversal plus the original 480-word destination permutation.
	Common::Array<uint8> output(static_cast<std::size_t>(kOutputBytes48), 0);
	Common::Array<uint8> selectedPixels(kSourceWidth48);
	int destinationRow = 0;
	for (int sourceRow = 0; sourceRow < kSourceHeight48; ++sourceRow) {
		if (!maskSelects(verticalScale, sourceRow)) {
			continue;
		}

		// Ghidra 0x0000D49A-0x0000D9F5: each of five vertical-mask words gates sixteen source rows; the
		// unrolled selected-row body tests all 48 bytes and compacts selected pixels into longwords.
		int selectedPixelCount = 0;
		int sourceRowOffset = sourceOffset + sourceRow * kSourceWidth48;
		for (int sourceColumn = 0; sourceColumn < kSourceWidth48; ++sourceColumn) {
			if (maskSelects(horizontalScale, sourceColumn)) {
				selectedPixels[static_cast<std::size_t>(selectedPixelCount++)] =
					decodedPixels[static_cast<std::size_t>(sourceRowOffset + sourceColumn)];
			}
		}

		for (int longword = 0; longword < kOutputLongwordsPerRow48; ++longword) {
			uint32 packedPixels = 0;
			int firstPixel = longword * 8;
			for (int pixel = 0; pixel < 8; ++pixel) {
				packedPixels <<= 4;
				int selectedPixel = firstPixel + pixel;
				if (selectedPixel < selectedPixelCount) {
					packedPixels |= selectedPixels[static_cast<std::size_t>(selectedPixel)];
				}
			}

			int destinationOffset = destinationRow < 32
										? destinationRow * 4 + longword * 0x80
									: destinationRow < 64
										? 0x300 + (destinationRow - 32) * 4 + longword * 0x80
										: 0x600 + (destinationRow - 64) * 4 + longword * 0x40;
			WriteUInt32BigEndian(output, destinationOffset, packedPixels);
		}

		// Ghidra 0x0000D9F6-0x0000DA37: left-align a partial longword, zero the remaining row, advance the
		// 48-byte source row, and repeat until all five sixteen-row groups are consumed.
		++destinationRow;
	}

	// Ghidra 0x0000DA38-0x0000DA69: explicitly clear all remaining destination rows. The output vector starts
	// zero-initialized, preserving the original complete 0x780-byte replacement.
	return output;
}

Common::Array<uint8> ActorSpriteTransformer::scaleCompactSprite80(
	const Common::Array<uint8> &decodedPixels,
	int sourceOffset,
	uint16 horizontalScale,
	uint16 verticalScale) const {
	// Ghidra 0x0000DA6A-0x0000DAA7: select the vertical and horizontal fixed-point mask records and
	// initialize the 48-row source traversal plus the original 480-word destination permutation.
	Common::Array<uint8> output(static_cast<std::size_t>(kOutputBytes80), 0);
	Common::Array<uint8> selectedPixels(kSourceWidth80);
	int destinationRow = 0;
	for (int sourceRow = 0; sourceRow < kSourceHeight80; ++sourceRow) {
		if (!maskSelects(verticalScale, sourceRow)) {
			continue;
		}

		// Ghidra 0x0000DAA8-0x0000E3AB: the unrolled body tests all eighty source bytes, compacts every
		// selected pixel into eight-nibble longwords, left-aligns a partial longword, and pads the row.
		int selectedPixelCount = 0;
		int sourceRowOffset = sourceOffset + sourceRow * kSourceWidth80;
		for (int sourceColumn = 0; sourceColumn < kSourceWidth80; ++sourceColumn) {
			if (maskSelects(horizontalScale, sourceColumn)) {
				selectedPixels[static_cast<std::size_t>(selectedPixelCount++)] =
					decodedPixels[static_cast<std::size_t>(sourceRowOffset + sourceColumn)];
			}
		}

		for (int longword = 0; longword < kOutputLongwordsPerRow80; ++longword) {
			uint32 packedPixels = 0;
			int firstPixel = longword * 8;
			for (int pixel = 0; pixel < 8; ++pixel) {
				packedPixels <<= 4;
				int selectedPixel = firstPixel + pixel;
				if (selectedPixel < selectedPixelCount) {
					packedPixels |= selectedPixels[static_cast<std::size_t>(selectedPixel)];
				}
			}

			int destinationOffset =
				destinationRow < 32
					? destinationRow * 4 + longword * 0x80
					: 0x500 + (destinationRow - 32) * 4 + longword * 0x40;
			WriteUInt32BigEndian(output, destinationOffset, packedPixels);
		}

		++destinationRow;
	}

	// Ghidra 0x0000E3AC-0x0000E40B: advance every source row and explicitly clear all unpublished destination
	// rows. The output vector starts zero-initialized and the selected-row loop replaces every published
	// longword, preserving the original complete 0x780-byte replacement.
	return output;
}

bool ActorSpriteTransformer::maskSelects(uint16 scale, int sourceIndex) const {
	int maskOffset = scale * kScaleMaskRecordSize + sourceIndex / 8;
	return (_scaleSelectionMasks[static_cast<std::size_t>(maskOffset)] & (0x80 >> sourceIndex % 8)) != 0;
}
} // namespace Scooby