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

#include "common/endian.h"

#include "tesoro/core/archive.h"

namespace Tesoro {

static int compareNames(const char *a, const char *b) {
	for (;; ++a, ++b) {
		const byte ca = (byte)*a, cb = (byte)*b;
		if (ca != cb)
			return ca < cb ? -1 : 1;
		if (ca == 0)
			return 0;
	}
}

void Archive::loadIndex(const byte *data, uint32 size) {
	_entries.clear();

	// Record: uint32 offset, uint32 (unused), flags byte, name[19].
	for (uint32 pos = 0; pos + kRecordSize <= size; pos += kRecordSize) {
		const byte *rec = data + pos;
		if ((rec[8] & 4) == 0) // skip those without the required flag
			continue;

		Entry e;
		memset(&e, 0, sizeof(e));
		for (uint32 i = 0; i < kNameSize && rec[9 + i]; ++i) {
			const char c = (char)rec[9 + i];
			e.name[i] = (c >= 'a' && c <= 'z') ? c - 0x20 : c; // to upper case
		}
		e.name[kNameSize - 1] = '\0';
		e.offset = READ_LE_UINT32(rec) + 4;
		_entries.push_back(e);
	}

	// Selection sort, largest name first
	const int n = (int)_entries.size();
	for (int i = 0; i < n - 1; ++i) {
		int best = i;
		for (int j = i + 1; j < n; ++j) {
			if (compareNames(_entries[j].name, _entries[best].name) > 0)
				best = j;
		}
		SWAP(_entries[i], _entries[best]);
	}
}

// binary search for an entry by name
uint32 Archive::findEntry(const char *name) const {
	int lo = 0, hi = (int)_entries.size() - 1;
	while (lo <= hi) {
		const int mid = (lo + hi) >> 1;
		const int c = compareNames(name, _entries[mid].name);
		if (c == 0)
			return _entries[mid].offset;
		if (c < 0)
			lo = mid + 1;
		else
			hi = mid - 1;
	}
	return 0;
}

bool Archive::open(const Common::Path &datName, const Common::Path &treName) {
	close();

	if (!treName.empty()) {
		Common::File tre;
		if (!tre.open(treName))
			return false;
		const uint32 size = (uint32)tre.size();
		byte *data = new byte[size];
		const uint32 got = tre.read(data, size);
		// builds the index from the TRE file data
		loadIndex(data, got);
		delete[] data;
	}

	return _dat.open(datName);
}

void Archive::close() {
	_dat.close();
}

Common::SeekableReadStream *Archive::seekEntry(uint32 offset) {
	_dat.seek(offset, SEEK_SET);
	return &_dat;
}

uint32 Archive::pos() const {
	return (uint32)_dat.pos();
}

uint32 Archive::seekSavePos(uint32 offset) {
	const uint32 previous = pos();
	_dat.seek(offset, SEEK_SET);
	return previous;
}

} // End of namespace Tesoro
