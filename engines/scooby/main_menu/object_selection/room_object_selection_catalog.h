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

#ifndef SCOOBY_ROOM_OBJECT_SELECTION_CATALOG_H
#define SCOOBY_ROOM_OBJECT_SELECTION_CATALOG_H

#include "common/scummsys.h"

#include "common/array.h"
#include "common/str.h"

#include "scooby/assets/rom.h"
#include "scooby/common/span.h"

// Decodes the episode-authored room-object labels consumed by the hidden catalog selector.
//
// Episode-zero records occupy 0x000FCA86-0x000FD4AF and contain 181 records, including 17 with
// the extended fixed-position payload. Episode-one records occupy 0x001505A8-0x00150E95 and
// contain 155 records, including 29 extended records. Fourteen-byte base records and eighteen-byte
// extended records must reach each descriptor's exclusive end exactly before labels are indexed
// through descriptor fields +0x20 and +0x28.

namespace Scooby {

class RoomObjectSelectionCatalog {
public:
	// Decodes both immutable room-object catalogs once from verified cartridge assets.
	// rom: verified cartridge address space containing descriptors, records, and labels.
	explicit RoomObjectSelectionCatalog(const ScoobyDooRom &rom);

	// Gets every authored label indexed identically to the active managed room-object array.
	// episodeIndex: managed equivalent of original episode word 0xFF06AA.
	// Returns all labels for the selected episode's room-object records.
	Span<const Common::String> getLabels(int episodeIndex) const {
		return MakeSpan(_episodeLabels[static_cast<std::size_t>(episodeIndex)]);
	}

private:
	// Original two-entry episode descriptor pointer table at 0x00031AF2-0x00031AF9.
	static const int kDescriptorTableOffset = 0x31AF2;
	static const int kDescriptorObjectEndField = 0x08;
	static const int kDescriptorObjectLabelBaseField = 0x20;
	static const int kDescriptorObjectLabelTableField = 0x28;
	static const int kDescriptorObjectStartField = 0x04;
	static const int kEpisodeCount = 2;
	static const int kExtendedSourceRecordSize = 18;
	static const int kObjectLabelRecordSize = 8;
	static const int kSourceFlagsField = 12;
	// BTST.B #1 at 0x00007FBA tests the high byte of this big-endian authored flags word.
	static const uint16 kSourceHasExtendedDataMask = 0x0200;
	static const int kSourceRecordSize = 14;

	static Common::Array<Common::String> decodeEpisode(const ScoobyDooRom &rom, int episodeIndex);
	static Common::String readNullTerminatedAscii(const ScoobyDooRom &rom, int offset);

	Common::Array<Common::Array<Common::String>> _episodeLabels;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_OBJECT_SELECTION_CATALOG_H
