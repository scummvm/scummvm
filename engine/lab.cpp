/* Residual - Virtual machine to run LucasArts' 3D adventure games
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

#include "common/sys.h"
#include "common/endian.h"
#include "common/file.h"

#include "engine/lab.h"

#include <algorithm>

bool Lab::open(const char *filename) {
	_labFileName = filename;

	close();
	_f = new Common::File();
	_f->open(filename);
	if (!_f->isOpen())
		return false;

	char header[16];
	if (_f->read(header, sizeof(header)) < sizeof(header)) {
		close();
		return false;
	}
	if (std::memcmp(header, "LABN", 4) != 0) {
		close();
		return false;
	}

	int num_entries = READ_LE_UINT32(header + 8);
	int string_table_size = READ_LE_UINT32(header + 12);

	char *string_table = new char[string_table_size];
	_f->seek(16 * (num_entries + 1), SEEK_SET);
	_f->read(string_table, string_table_size);

	_f->seek(16, SEEK_SET);
	char binary_entry[16];
	for (int i = 0; i < num_entries; i++) {
		_f->read(binary_entry, 16);
		int fname_offset = READ_LE_UINT32(binary_entry);
		int start = READ_LE_UINT32(binary_entry + 4);
		int size = READ_LE_UINT32(binary_entry + 8);

		std::string fname = string_table + fname_offset;
		std::transform(fname.begin(), fname.end(), fname.begin(), tolower);

		_fileMap.insert(std::make_pair(fname, LabEntry(start, size)));
		_fileMap.size();
	}

	delete []string_table;
	return true;
}

bool Lab::fileExists(const char *filename) const {
	return findFilename(filename) != _fileMap.end();
}

Block *Lab::getFileBlock(const char *filename) const {
	FileMapType::const_iterator i = findFilename(filename);
	if (i == _fileMap.end())
		return NULL;

	_f->seek(i->second.offset, SEEK_SET);
	char *data = new char[i->second.len];
	_f->read(data, i->second.len);
	return new Block(data, i->second.len);
}

LuaFile *Lab::openNewStreamLua(const char *filename) const {
	FileMapType::const_iterator i = findFilename(filename);
	if (i == _fileMap.end())
		return NULL;

	Common::File *file = new Common::File();
	file->open(_labFileName.c_str());
	file->seek(i->second.offset, SEEK_SET);

	LuaFile *filehandle = new LuaFile();
	filehandle->_file = file;

	return filehandle;
}

Common::File *Lab::openNewStreamFile(const char *filename) const {
	Common::File *file;
	FileMapType::const_iterator i = findFilename(filename);
	if (i == _fileMap.end())
		return NULL;

	file = new Common::File();
	file->open(_labFileName.c_str());
	file->seek(i->second.offset, SEEK_SET);

	return file;
}

int Lab::fileLength(const char *filename) const {
	FileMapType::const_iterator i = findFilename(filename);
	if (i == _fileMap.end())
		return -1;

	return i->second.len;
}

Lab::FileMapType::const_iterator Lab::findFilename(const char *filename) const {
	std::string s = filename;
	std::transform(s.begin(), s.end(), s.begin(), tolower);
	return _fileMap.find(s);
}

void Lab::close() {
	if (_f)
		delete _f;
	_f = NULL;
	_fileMap.clear();
}
