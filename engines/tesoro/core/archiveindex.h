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

#ifndef TESORO_CORE_ARCHIVEINDEX_H
#define TESORO_CORE_ARCHIVEINDEX_H

#include "common/array.h"
#include "common/scummsys.h"

namespace Tesoro {

/**
 * Directory of the data file, built from the index (.TRE) file.
 * The entries are kept sorted by name in descending order, which is the order find() searches.
 */
class ArchiveIndex {
public:
	static const uint32 kRecordSize = 28;
	static const uint32 kNameSize = 19;

	struct Entry {
		char name[kNameSize];	// upper case, at most 18 characters
		uint32 offset;			// start of the entry's payload in the data file
	};

	void load(const byte *data, uint32 size);

	uint32 count() const { return _entries.size(); }
	const Entry &entry(uint32 i) const { return _entries[i]; }

	/** Exact, case sensitive lookup. Returns the entry's data offset, or 0 if it does not exist. */
	uint32 find(const char *name) const;

private:
	Common::Array<Entry> _entries;
};

} // End of namespace Tesoro

#endif
