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

#include "episode_initializer.h"

#include "common/scummsys.h"
#include "common/algorithm.h"

namespace Scooby {

void EpisodeInitializer::initialize(int episodeIndex) {
	// Ghidra 0x00007F22-0x00007F33: resolve the selected descriptor through its two-entry pointer table.
	int descriptorOffset =
		static_cast<int>(_rom.readUInt32(
			kDescriptorTableOffset + episodeIndex * static_cast<int>(sizeof(uint32))));

	// Ghidra 0x00007F34-0x00007F45: clear all 0x100 progress bytes at original RAM 0xFF2A00.
	Common::fill(_state.ProgressStateBytes.begin(), _state.ProgressStateBytes.end(), 0);

	// Ghidra 0x00007F46-0x00007F59: copy the descriptor's authored [start, end) progress range.
	int progressStartOffset = static_cast<int>(_rom.readUInt32(descriptorOffset));
	int progressEndOffset = static_cast<int>(_rom.readUInt32(descriptorOffset + static_cast<int>(sizeof(uint32))));
	Span<const uint8> progressBytes =
		_rom.readBytes(progressStartOffset, progressEndOffset - progressStartOffset);
	Common::copy(progressBytes.begin(), progressBytes.end(), _state.ProgressStateBytes.begin());

	// Ghidra 0x00007F5A-0x00007F64: follow descriptor +0x30 and publish its initial room word.
	int initialRoomOffset = static_cast<int>(_rom.readUInt32(descriptorOffset + kInitialRoomPointerOffset));
	_state.RoomId = _rom.readInt16(initialRoomOffset);
}
} // namespace Scooby