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

#include "room_script_command_1a_executor.h"

#include "common/scummsys.h"

#include "scooby/common/span.h"

namespace Scooby {

void RoomScriptCommand1AExecutor::executeRoomScriptCommand1A(int mode) {
	int32 commandOffset = _state.RoomScriptStartOffset;

	// Ghidra 0x00002626-0x00002629: scan mode skips every selector, configuration, and palette effect.
	if (mode != 0) {
		// Ghidra 0x0000262A-0x00002635: the signed comparison skips only selectors greater than four; negative
		// selectors retain the native disable or configuration path without a lower-bound guard.
		int16 rangeIndex = _rom.readInt16(commandOffset + 2);
		if (rangeIndex <= 4) {
			int16 interval = _rom.readInt16(commandOffset + 8);
			uint8 rangeMask = static_cast<uint8>(1 << (rangeIndex & 0x07));
			if (interval < 0) {
				// Ghidra 0x00002636-0x0000265F: disable the modulo-eight selector bit, retain the doubled-word
				// source offset and unsigned end-minus-start count, and replace CRAM DMA with publication of
				// the same logical target-palette slice.
				_state.AnimatedPaletteRangeFlags &= static_cast<uint8>(~rangeMask);
				uint16 startIndex = _rom.readUInt16(commandOffset + 4);
				int16 paletteIndexShifted = static_cast<int16>(startIndex << 1);
				int paletteIndex = paletteIndexShifted / static_cast<int>(sizeof(uint16));
				uint16 colorCount =
					static_cast<uint16>(_rom.readUInt16(commandOffset + 6) - startIndex);
				_scene.replacePaletteRange(MakeSpan(_state.TargetPalette)
											   .slice(static_cast<std::size_t>(paletteIndex),
													  static_cast<std::size_t>(colorCount)),
										   paletteIndex);
			} else {
				// Ghidra 0x00002660-0x00002687: write the selected start and end, initialize the live
				// countdown before its identical reload interval, then enable the bit after every field.
				_state.AnimatedPaletteRangeStartIndices[static_cast<std::size_t>(rangeIndex)] =
					_rom.readUInt16(commandOffset + 4);
				_state.AnimatedPaletteRangeEndIndices[static_cast<std::size_t>(rangeIndex)] =
					_rom.readUInt16(commandOffset + 6);
				_state.AnimatedPaletteRangeCountdowns[static_cast<std::size_t>(rangeIndex)] = interval;
				_state.AnimatedPaletteRangeIntervals[static_cast<std::size_t>(rangeIndex)] = interval;
				_state.AnimatedPaletteRangeFlags |= rangeMask;
			}
		}
	}

	// Ghidra 0x00002688-0x0000268D: every native mode, selector, and control branch consumes ten bytes.
	_state.RoomScriptStartOffset = commandOffset + 10;
}
} // namespace Scooby