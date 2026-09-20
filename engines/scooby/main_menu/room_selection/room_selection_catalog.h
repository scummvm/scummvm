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

#ifndef SCOOBY_ROOM_SELECTION_CATALOG_H
#define SCOOBY_ROOM_SELECTION_CATALOG_H

#include "common/scummsys.h"

#include "common/array.h"
#include "common/str.h"

#include "scooby/assets/rom.h"
#include "scooby/common/span.h"

// Decodes the two authored room-name catalogs consumed by the hidden menu selector.
//
// Descriptor table 0x00031AF2-0x00031AF9 selects 20-byte record ranges
// [0x000FD4B0,0x000FD654) and [0x00150E96,0x001510EE), producing maximum room IDs 21 and 30.
// IDs zero and one use fixed labels at 0x00008A9C and 0x00008AA2; later IDs use descriptor
// field +0x20. The terminal record in each range is deliberately excluded from selectable labels.

namespace Scooby {

class RoomSelectionCatalog {
public:
	// Decodes both immutable catalogs once from the verified cartridge content.
	// rom: verified cartridge address space containing descriptors, records, and labels.
	explicit RoomSelectionCatalog(const ScoobyDooRom &rom);

	// Gets every selectable label indexed by its room identifier.
	// episodeIndex: managed equivalent of original episode word 0xFF06AA.
	// Returns labels for room IDs zero through the descriptor's maximum selectable ID.
	Span<const Common::String> getRoomLabels(int episodeIndex) const {
		return MakeSpan(_episodeRoomLabels[static_cast<std::size_t>(episodeIndex)]);
	}

private:
	// Original two-entry episode descriptor pointer table at 0x00031AF2-0x00031AF9.
	static const int kDescriptorTableOffset = 0x31AF2;
	static const int kDescriptorRoomNameBaseField = 0x20;
	static const int kDescriptorRoomRecordEndField = 0x0C;
	static const int kDescriptorRoomRecordStartField = 0x08;
	static const int kEpisodeCount = 2;

	// Original fixed labels at 0x00008A9C-0x00008AA1 and 0x00008AA2-0x00008AB2.
	static const int kLimboLabelOffset = 0x8A9C;
	static const int kPlayerInventoryLabelOffset = 0x8AA2;

	// Ghidra SelectRoomFromCatalog uses 20-byte records at 0x000088E8 and 0x00008956.
	static const int kRoomRecordSize = 0x14;

	static Common::Array<Common::String> decodeEpisode(const ScoobyDooRom &rom, int episodeIndex,
													   const Common::String &limboLabel,
													   const Common::String &playerInventoryLabel);
	static Common::String readNullTerminatedAscii(const ScoobyDooRom &rom, int offset);

	Common::Array<Common::Array<Common::String>> _episodeRoomLabels;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_SELECTION_CATALOG_H
