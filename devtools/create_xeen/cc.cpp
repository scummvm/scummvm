/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

 // Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

// HACK to allow building with the SDL backend on MinGW
// see bug #1800764 "TOOLS: MinGW tools building broken"
#ifdef main
#undef main
#endif // main

#include "cc.h"
#include "common/endian.h"

uint16 CCArchive::convertNameToId(const Common::String &resourceName) {
	if (resourceName.empty())
		return 0xffff;

	Common::String name = resourceName;
	name.toUppercase();

	// Check if a resource number is being directly specified
	if (name.size() == 4) {
		char *endPtr;
		uint16 num = (uint16)strtol(name.c_str(), &endPtr, 16);
		if (!*endPtr)
			return num;
	}

	const byte *msgP = (const byte *)name.c_str();
	int total = *msgP++;
	for (; *msgP; total += *msgP++) {
		// Rotate the bits in 'total' right 7 places
		total = (total & 0x007F) << 9 | (total & 0xFF80) >> 7;
	}

	return total;
}

void CCArchive::saveIndex() {
	// Fill up the data for the index entries into a raw data block
	byte *rawIndex = new byte[_index.size() * 8];
	byte b;

	byte *entryP = rawIndex;
	uint entryOffset = 8 * _index.size() + 2;
	for (uint i = 0; i < _index.size(); ++i, entryP += 8) {
		CCEntry &entry = _index[i];
		entry._offset = entryOffset;
		entryOffset += entry._size;

		WRITE_LE_UINT16(&entryP[0], entry._id);
		WRITE_LE_UINT32(&entryP[2], entry._offset);
		WRITE_LE_UINT16(&entryP[5], entry._size);
		entryP[7] = 0;
	}

	// Encrypt the index
	int seed = 0xac;
	for (uint i = 0; i < _index.size() * 8; ++i, seed += 0x67) {
		b = (rawIndex[i] - seed) & 0xff;
		b = (byte)((b >> 2) | (b << 6));

		assert(rawIndex[i] == (byte)((((b << 2) | (b >> 6)) + seed) & 0xff));
		rawIndex[i] = b;
	}

	// Write out the number of entries and the encrypted index data
	_file.writeWord(_index.size());
	_file.write(rawIndex, _index.size() * 8);

	delete[] rawIndex;
}

void CCArchive::saveEntries() {
	for (uint idx = 0; idx < _index.size(); ++idx) {
		CCEntry &ccEntry = _index[idx];

		// Encrypt the entry
		for (int i = 0; i < ccEntry._size; ++i)
			ccEntry._data[i] ^= 0x35;

		// Write out the entry
		_file.seek(ccEntry._offset);
		_file.write(ccEntry._data, ccEntry._size);
	}
}

void CCArchive::close() {
	saveIndex();
	saveEntries();
}

void CCArchive::add(const Common::String &name, Common::MemFile &f) {
	_index.push_back(CCEntry(convertNameToId(name), f.getData(), f.size()));
}
