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

static int sortCallback(const void *entry1, const void *entry2) {
	return strcasecmp(((Lab::LabEntry *)entry1)->filename, ((Lab::LabEntry *)entry2)->filename);
}

bool Lab::open(const char *filename) {
	_labFileName = filename;

	close();
	_f = new Common::File();
	_f->open(filename);
	if (!_f->isOpen())
		return false;

	char header[16];
	_f->read(header, sizeof(header));
	if (READ_BE_UINT32(header) != MKID_BE('LABN')) {
		close();
		return false;
	}

	_numEntries = READ_LE_UINT32(header + 8);
	int string_table_size = READ_LE_UINT32(header + 12);

	char *string_table = new char[string_table_size];
	_f->seek(16 * (_numEntries + 1), SEEK_SET);
	_f->read(string_table, string_table_size);

	_entries = new LabEntry[_numEntries];
	_f->seek(16, SEEK_SET);
	char binary_entry[16];
	for (int i = 0; i < _numEntries; i++) {
		_f->read(binary_entry, 16);
		int fname_offset = READ_LE_UINT32(binary_entry);
		int start = READ_LE_UINT32(binary_entry + 4);
		int size = READ_LE_UINT32(binary_entry + 8);

		Common::String fname = string_table + fname_offset;
		fname.toLowercase();

		_entries[i].offset = start;
		_entries[i].len = size;
		_entries[i].filename = new char[fname.size() + 1];
		strcpy(_entries[i].filename, fname.c_str());
	}

	qsort(_entries, _numEntries, sizeof(LabEntry), sortCallback);

	delete[] string_table;
	return true;
}

bool Lab::fileExists(const char *filename) const {
	return findFilename(filename) != NULL;
}

bool Lab::isOpen() const {
	return _f && _f->isOpen();
}

Block *Lab::getFileBlock(const char *filename) const {
	LabEntry *i = findFilename(filename);
	if (!i)
		return NULL;

	_f->seek(i->offset, SEEK_SET);
	char *data = new char[i->len];
	_f->read(data, i->len);
	return new Block(data, i->len);
}

LuaFile *Lab::openNewStreamLua(const char *filename) const {
	LabEntry *i = findFilename(filename);
	if (!i)
		return NULL;

	Common::File *file = new Common::File();
	file->open(_labFileName);
	file->seek(i->offset, SEEK_SET);

	LuaFile *filehandle = new LuaFile();
	filehandle->_in = file;

	return filehandle;
}

Common::File *Lab::openNewStreamFile(const char *filename) const {
	Common::File *file;
	LabEntry *i = findFilename(filename);
	if (!i)
		return NULL;

	file = new Common::File();
	file->open(_labFileName.c_str());
	file->seek(i->offset, SEEK_SET);

	return file;
}

int Lab::fileLength(const char *filename) const {
	LabEntry *i = findFilename(filename);
	if (!i)
		return -1;

	return i->len;
}

Lab::LabEntry *Lab::findFilename(const char *filename) const {
	LabEntry key;

	Common::String s = filename;
	s.toLowercase();
	key.filename = (char *)s.c_str();

	return (Lab::LabEntry *)bsearch(&key, _entries, _numEntries, sizeof(LabEntry), sortCallback);
}

void Lab::close() {
	delete _f;
	_f = NULL;
	if (_entries)
		for (int i = 0; i < _numEntries; i++)
			delete[] _entries[i].filename;

	delete[] _entries;
	_entries = NULL;
	_numEntries = 0;
}

} // end of namespace Grim
