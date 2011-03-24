/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include "common/endian.h"
#include "common/file.h"

#include "engines/grim/lab.h"
#include "engines/grim/lua/lua.h"

namespace Grim {

bool Lab::open(const Common::String &filename) {
	_labFileName = filename;

	close();

	_f = new Common::File();
	if (!_f->open(filename))
		return false;

	char header[16];
	_f->read(header, sizeof(header));
	if (READ_BE_UINT32(header) != MKID_BE('LABN')) {
		close();
		return false;
	}

	uint32 entryCount = READ_LE_UINT32(header + 8);

	int string_table_size = READ_LE_UINT32(header + 12);
	char *string_table = new char[string_table_size];
	_f->seek(16 * (entryCount + 1));
	_f->read(string_table, string_table_size);
	_f->seek(16);

	for (uint32 i = 0; i < entryCount; i++) {
		int fname_offset = _f->readUint32LE();
		int start = _f->readUint32LE();
		int size = _f->readUint32LE();
		_f->readUint32LE();

		Common::String fname = string_table + fname_offset;

		LabEntry entry;
		entry.offset = start;
		entry.len = size;

		_entries[fname] = entry;
	}

	delete[] string_table;
	return true;
}

bool Lab::fileExists(const Common::String &filename) const {
	return _entries.contains(filename);
}

bool Lab::isOpen() const {
	return _f && _f->isOpen();
}

Block *Lab::getFileBlock(const Common::String &filename) const {
	if (!fileExists(filename))
		return 0;

	const LabEntry &i = _entries[filename];

	_f->seek(i.offset, SEEK_SET);
	char *data = new char[i.len];
	_f->read(data, i.len);
	return new Block(data, i.len);
}

LuaFile *Lab::openNewStreamLua(const Common::String &filename) const {
	if (!fileExists(filename))
		return 0;

	Common::File *file = new Common::File();
	file->open(_labFileName);
	file->seek(_entries[filename].offset, SEEK_SET);

	LuaFile *filehandle = new LuaFile();
	filehandle->_in = file;

	return filehandle;
}

Common::File *Lab::openNewStreamFile(const Common::String &filename) const {
	if (!fileExists(filename))
		return 0;

	Common::File *file = new Common::File();
	file->open(_labFileName);
	file->seek(_entries[filename].offset, SEEK_SET);

	return file;
}

int Lab::fileLength(const Common::String &filename) const {
	if (!fileExists(filename))
		return -1;

	return _entries[filename].len;
}

void Lab::close() {
	delete _f;
	_f = NULL;

	_entries.clear();
}

} // end of namespace Grim
