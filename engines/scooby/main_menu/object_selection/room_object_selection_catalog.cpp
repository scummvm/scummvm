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

#include "room_object_selection_catalog.h"

namespace Scooby {

RoomObjectSelectionCatalog::RoomObjectSelectionCatalog(const ScoobyDooRom &rom)
	: _episodeLabels(kEpisodeCount) {
	for (int episodeIndex = 0; episodeIndex < static_cast<int>(_episodeLabels.size()); episodeIndex++) {
		_episodeLabels[static_cast<std::size_t>(episodeIndex)] = decodeEpisode(rom, episodeIndex);
	}
}

Common::Array<Common::String> RoomObjectSelectionCatalog::decodeEpisode(
	const ScoobyDooRom &rom, int episodeIndex) {
	int descriptorOffset =
		static_cast<int>(rom.readUInt32(kDescriptorTableOffset + episodeIndex * static_cast<int>(sizeof(uint32))));
	int sourceOffset = static_cast<int>(rom.readUInt32(descriptorOffset + kDescriptorObjectStartField));
	int sourceEndOffset = static_cast<int>(rom.readUInt32(descriptorOffset + kDescriptorObjectEndField));
	int objectCount = 0;
	while (sourceOffset < sourceEndOffset) {
		uint16 flags = rom.readUInt16(sourceOffset + kSourceFlagsField);
		sourceOffset += (flags & kSourceHasExtendedDataMask) != 0
							? kExtendedSourceRecordSize
							: kSourceRecordSize;
		objectCount++;
	}

	int labelBaseOffset = static_cast<int>(rom.readUInt32(
		descriptorOffset + kDescriptorObjectLabelBaseField));
	int labelTableOffset = static_cast<int>(rom.readUInt32(
		descriptorOffset + kDescriptorObjectLabelTableField));
	Common::Array<Common::String> labels(static_cast<std::size_t>(objectCount));
	for (int objectIndex = 0; objectIndex < static_cast<int>(labels.size()); objectIndex++) {
		int recordOffset = labelTableOffset + objectIndex * kObjectLabelRecordSize;
		int labelOffset =
			labelBaseOffset + static_cast<int>(rom.readUInt32(
								  recordOffset + static_cast<int>(sizeof(uint32))));
		labels[static_cast<std::size_t>(objectIndex)] = readNullTerminatedAscii(rom, labelOffset);
	}

	return labels;
}

Common::String RoomObjectSelectionCatalog::readNullTerminatedAscii(const ScoobyDooRom &rom, int offset) {
	int endOffset = offset;
	while (rom.readByte(endOffset) != 0) {
		endOffset++;
	}

	Span<const uint8> bytes = rom.readBytes(offset, endOffset - offset);
	return Common::String(reinterpret_cast<const char *>(bytes.data()), static_cast<uint32>(bytes.size()));
}
} // namespace Scooby