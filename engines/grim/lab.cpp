/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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
 */

#include "common/endian.h"
#include "common/file.h"
#include "common/substream.h"

#include "engines/grim/grim.h"
#include "engines/grim/lab.h"
#include "engines/grim/lua/lua.h"
#include "engines/grim/colormap.h"

namespace Grim {

bool Lab::open(const Common::String &filename) {
	_labFileName = filename;

	close();

	_f = new Common::File();
	if (!_f->open(filename))
		return false;

	if (_f->readUint32BE() != MKTAG('L','A','B','N')) {
		close();
		return false;
	}

	_f->readUint32LE(); // version

	if (g_grim->getGameType() == GType_GRIM)
		parseGrimFileTable();
	else
		parseMonkey4FileTable();

	return true;
}

void Lab::parseGrimFileTable() {
	uint32 entryCount = _f->readUint32LE();
	uint32 stringTableSize = _f->readUint32LE();

	char *stringTable = new char[stringTableSize];
	_f->seek(16 * (entryCount + 1));
	_f->read(stringTable, stringTableSize);
	_f->seek(16);

	for (uint32 i = 0; i < entryCount; i++) {
		int fnameOffset = _f->readUint32LE();
		int start = _f->readUint32LE();
		int size = _f->readUint32LE();
		_f->readUint32LE();

		Common::String fname = stringTable + fnameOffset;

		LabEntry entry;
		entry.offset = start;
		entry.len = size;

		_entries[fname] = entry;
	}

	delete[] stringTable;
}

void Lab::parseMonkey4FileTable() {
	uint32 entryCount = _f->readUint32LE();
	uint32 stringTableSize = _f->readUint32LE();
	uint32 stringTableOffset = _f->readUint32LE() - 0x13d0f;

	char *stringTable = new char[stringTableSize];
	_f->seek(stringTableOffset);
	_f->read(stringTable, stringTableSize);
	_f->seek(20);

	// Decrypt the string table
	for (uint32 i = 0; i < stringTableSize; i++)
		if (stringTable[i] != 0)
			stringTable[i] ^= 0x96;

	for (uint32 i = 0; i < entryCount; i++) {
		int fnameOffset = _f->readUint32LE();
		int start = _f->readUint32LE();
		int size = _f->readUint32LE();
		_f->readUint32LE();

		char *str = stringTable + fnameOffset;
		int len = strlen(str);

		for (int l = 0; l < len; ++l) {
			if (str[l] == '\\')
				str[l] = '/';
		}
		Common::String fname = str;

		LabEntry entry;
		entry.offset = start;
		entry.len = size;

		_entries[fname] = entry;
	}

	delete[] stringTable;
}

bool Lab::getFileExists(const Common::String &filename) const {
	return _entries.contains(filename);
}

bool Lab::isOpen() const {
	return _f && _f->isOpen();
}

Block *Lab::getFileBlock(const Common::String &filename) const {
	if (!getFileExists(filename))
		return 0;

	const LabEntry &i = _entries[filename];

	_f->seek(i.offset, SEEK_SET);
	char *data = new char[i.len];
	_f->read(data, i.len);
	return new Block(data, i.len);
}

LuaFile *Lab::openNewStreamLua(const Common::String &filename) const {
	if (!getFileExists(filename))
		return 0;

	Common::File *file = new Common::File();
	file->open(_labFileName);
	file->seek(_entries[filename].offset, SEEK_SET);

	LuaFile *filehandle = new LuaFile();
	filehandle->_in = file;

	return filehandle;
}

Common::File *Lab::openNewStreamFile(const Common::String &filename) const {
	if (!getFileExists(filename))
		return 0;

	Common::File *file = new Common::File();
	file->open(_labFileName);
	file->seek(_entries[filename].offset, SEEK_SET);

	return file;
}
// SubStream, for usage with GZipReadStream
Common::SeekableReadStream *Lab::openNewSubStreamFile(const Common::String &filename) const {
	if (!getFileExists(filename))
		return 0;

	Common::File *file = new Common::File();
	file->open(_labFileName);
	Common::SeekableSubReadStream *substream;
	substream = new Common::SeekableSubReadStream(file, _entries[filename].offset, _entries[filename].len, DisposeAfterUse::YES );
	return substream;
}

int Lab::getFileLength(const Common::String &filename) const {
	if (!getFileExists(filename))
		return -1;

	return _entries[filename].len;
}

void Lab::close() {
	delete _f;
	_f = NULL;

	_entries.clear();
}

} // end of namespace Grim
