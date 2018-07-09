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

#include "archive.h"
#include "common/endian.h"

#define ARCHIVE_IDENT "SULT"
#define ARCHIVE_VERSION 1

void Archive::save() {
	// Write identifying string and archive version
	_file.write(ARCHIVE_IDENT, 4);
	_file.writeWord(ARCHIVE_VERSION);
	_file.writeWord(0);

	// Figure out the size the entire index needs to be (i.e. where the data starts)
	size_t dataOffset = 8;
	for (uint idx = 0; idx < _index.size(); ++idx)
		dataOffset += _index[idx].getIndexSize();
	dataOffset = ((dataOffset + 1) / 2) * 2;

	// Iterate through writing out index entries
	for (uint idx = 0; idx < _index.size(); ++idx) {
		ArchiveEntry &ae = _index[idx];
		ae._offset = dataOffset;
		_file.writeString(ae._name.c_str());
		_file.writeLong(ae._offset);
		_file.writeWord(ae._size);
	}
	if (_file.pos() % 2)
		_file.writeByte(0);

	// Write out the contents of each resource
	for (uint idx = 0; idx < _index.size(); ++idx) {
		const ArchiveEntry &ae = _index[idx];
		if (_file.pos() != ae._offset)
			error("Incorrect offset");
		_file.write(ae._data, ae._size);
	}
}

bool Archive::open(const Common::String &name) {
	return _file.open(name.c_str(), Common::kFileWriteMode);
}

void Archive::close() {
	if (_file.isOpen()) {
		save();
		_file.close();
	}
}

void Archive::add(const Common::String &name, Common::MemFile &f) {
	_index.push_back(ArchiveEntry(name, f.getData(), f.size()));
}
