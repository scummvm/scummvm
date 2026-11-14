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

#include "scooby/main_menu/reveal/compositor.h"

#include "common/scummsys.h"

namespace Scooby {
namespace {

const int kMaskTableOffset = 0xC3F0;
const int kMaskStride = 16;
const int kNarrowOutputOffsetTable = 0xBA70;
const int kNarrowSourceHeight = 32;
const int kNarrowSourceWidth = 64;
const int kRowShiftCount = 39;
const int kRowShiftTableOffset = 0x8828;
const int kWideOutputOffsetTable = 0xB5C0;
const int kWideSourceHeight = 40;
const int kWideSourceWidth = 120;

bool maskSelects(const ScoobyDooRom &rom, int maskOffset, int index) {
	return (rom.readByte(maskOffset + index / 8) & (0x80 >> (index % 8))) != 0;
}

void packRow(const ScoobyDooRom &rom, Span<const uint8> selectedPixels,
			 int selectedPixelCount, Common::Array<uint8> &output, int destinationRow,
			 int longwordsPerRow,
			 int outputOffsetTable) {
	for (int group = 0; group < longwordsPerRow; group++) {
		uint32 packedPixels = 0;
		int firstPixel = group * 8;
		for (int pixel = 0; pixel < 8; pixel++) {
			packedPixels <<= 4;
			int selectedIndex = firstPixel + pixel;
			if (selectedIndex < selectedPixelCount) {
				packedPixels |= selectedPixels[static_cast<size_t>(selectedIndex)];
			}
		}

		int offsetIndex = destinationRow * longwordsPerRow + group;
		uint16 destinationOffset = rom.readUInt16(outputOffsetTable + offsetIndex *
																		  static_cast<int>(sizeof(uint16)));
		writeUInt32BigEndian(output, destinationOffset, packedPixels);
	}
}

Common::Array<uint8> compose(const ScoobyDooRom &rom,
							 Span<const uint8> source,
							 int16 rowMaskOffset, int16 columnMaskOffset,
							 int rowShiftStartIndex, int sourceWidth, int sourceHeight,
							 int rowShiftDirection,
							 int outputOffsetTable) {
	int longwordsPerRow = sourceWidth / 8;
	Common::Array<uint8> output(static_cast<size_t>(sourceWidth * sourceHeight / 2));
	Common::Array<uint8> selectedPixels(static_cast<size_t>(sourceWidth));
	int rowMask = kMaskTableOffset + rowMaskOffset * kMaskStride;
	int columnMask = kMaskTableOffset + columnMaskOffset * kMaskStride;
	int destinationRow = 0;
	int shiftIndex = rowShiftStartIndex;
	for (int sourceRow = 0; sourceRow < sourceHeight; sourceRow++) {
		int rowShift = rom.readUInt16(
						   kRowShiftTableOffset + shiftIndex * static_cast<int>(sizeof(uint16))) *
					   rowShiftDirection;
		shiftIndex = (shiftIndex + 1) % kRowShiftCount;
		if (!maskSelects(rom, rowMask, sourceRow)) {
			continue;
		}

		int selectedPixelCount = 0;
		int shiftedRowStart = sourceRow * sourceWidth + rowShift;
		for (int sourceColumn = 0; sourceColumn < sourceWidth; sourceColumn++) {
			if (!maskSelects(rom, columnMask, sourceColumn)) {
				continue;
			}

			int sourceIndex = shiftedRowStart + sourceColumn;
			selectedPixels[static_cast<size_t>(selectedPixelCount++)] =
				static_cast<size_t>(sourceIndex) < source.size()
					? source[static_cast<size_t>(sourceIndex)]
					: static_cast<uint8>(0);
		}

		packRow(rom, makeSpan(selectedPixels), selectedPixelCount, output, destinationRow++,
				longwordsPerRow,
				outputOffsetTable);
	}

	return output;
}
} // namespace

Common::Array<uint8> composeWide(const ScoobyDooRom &rom,
								 Span<const uint8> source,
								 int16 rowMaskOffset, int16 columnMaskOffset,
								 int rowShiftStartIndex) {
	return compose(rom, source, rowMaskOffset, columnMaskOffset, rowShiftStartIndex, kWideSourceWidth,
				   kWideSourceHeight, -1, kWideOutputOffsetTable);
}

Common::Array<uint8> composeNarrow(const ScoobyDooRom &rom,
								   Span<const uint8> source,
								   int16 rowMaskOffset, int16 columnMaskOffset,
								   int rowShiftStartIndex) {
	return compose(rom, source, rowMaskOffset, columnMaskOffset, rowShiftStartIndex, kNarrowSourceWidth,
				   kNarrowSourceHeight, 1, kNarrowOutputOffsetTable);
}
} // namespace Scooby
