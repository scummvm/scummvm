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

#ifndef TESORO_CORE_ARCHIVE_H
#define TESORO_CORE_ARCHIVE_H

#include "common/array.h"
#include "common/file.h"

namespace Tesoro {

/**
 * The game's data file (.DAT) together with its directory, built from the index (.TRE) file.
 * The entries are kept sorted by name in descending order, which is the order findEntry() searches.
 */
class Archive {
public:

	bool open(const Common::Path &datName, const Common::Path &treName);
	void close();

	/** Retrieves the data offset of an entry (exact, case sensitive lookup), 0 if missing. Names are upper case. */
	uint32 findEntry(const char *name) const;

	/** Positions the data file at an offset from findEntry() and returns it for reading. */
	Common::SeekableReadStream *seekEntry(uint32 offset);

	uint32 pos() const;

	/** Seeks to offset keeping the previous position. */
	uint32 seekSavePos(uint32 offset);

private:
	static const uint32 kRecordSize = 28;
	static const uint32 kNameSize = 19;

	struct Entry {
		char name[kNameSize];	// upper case, at most 18 characters
		uint32 offset;			// start of the entry's payload in the data file
	};

	void loadIndex(const byte *data, uint32 size);

	Common::Array<Entry> _entries;
	Common::File _dat;
};

} // End of namespace Tesoro

#endif
