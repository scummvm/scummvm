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

#include "room_selection_catalog.h"

namespace Scooby {

RoomSelectionCatalog::RoomSelectionCatalog(const ScoobyDooRom &rom)
	: _episodeRoomLabels(kEpisodeCount) {
	Common::String limboLabel = readNullTerminatedAscii(rom, kLimboLabelOffset);
	Common::String playerInventoryLabel = readNullTerminatedAscii(rom, kPlayerInventoryLabelOffset);
	for (int episodeIndex = 0; episodeIndex < static_cast<int>(_episodeRoomLabels.size()); episodeIndex++) {
		_episodeRoomLabels[static_cast<std::size_t>(episodeIndex)] =
			decodeEpisode(rom, episodeIndex, limboLabel, playerInventoryLabel);
	}
}

Common::Array<Common::String> RoomSelectionCatalog::decodeEpisode(
	const ScoobyDooRom &rom, int episodeIndex,
	const Common::String &limboLabel,
	const Common::String &playerInventoryLabel) {
	int descriptorOffset =
		static_cast<int>(rom.readUInt32(kDescriptorTableOffset + episodeIndex * static_cast<int>(sizeof(uint32))));
	int recordStartOffset = static_cast<int>(rom.readUInt32(
		descriptorOffset + kDescriptorRoomRecordStartField));
	int recordEndOffset = static_cast<int>(rom.readUInt32(descriptorOffset + kDescriptorRoomRecordEndField));
	int nameBaseOffset = static_cast<int>(rom.readUInt32(descriptorOffset + kDescriptorRoomNameBaseField));
	int maximumRoomId = (recordEndOffset - recordStartOffset) / kRoomRecordSize;

	Common::Array<Common::String> labels(static_cast<std::size_t>(maximumRoomId + 1));
	labels[0] = limboLabel;
	labels[1] = playerInventoryLabel;
	for (int roomId = 2; roomId <= maximumRoomId; roomId++) {
		int recordOffset = recordStartOffset + (roomId - 2) * kRoomRecordSize;
		int labelOffset = nameBaseOffset + static_cast<int>(rom.readUInt32(recordOffset));
		labels[static_cast<std::size_t>(roomId)] = readNullTerminatedAscii(rom, labelOffset);
	}

	return labels;
}

Common::String RoomSelectionCatalog::readNullTerminatedAscii(const ScoobyDooRom &rom, int offset) {
	int endOffset = offset;
	while (rom.readByte(endOffset) != 0) {
		endOffset++;
	}

	Span<const uint8> bytes = rom.readBytes(offset, endOffset - offset);
	return Common::String(reinterpret_cast<const char *>(bytes.data()), static_cast<uint32>(bytes.size()));
}
} // namespace Scooby