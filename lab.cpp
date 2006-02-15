// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#include "stdafx.h"
#include "bits.h"
#include "lab.h"

#include <algorithm>
#include <cstdlib>
#include <cctype>

bool Lab::open(const char *filename) {
	_labFileName = filename;

	close();
	_f = std::fopen(filename, "rb");
	if (!isOpen())
		return false;

	char header[16];
	if (std::fread(header, 1, sizeof(header), _f) < sizeof(header)) {
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
	std::fseek(_f, 16 * (num_entries + 1), SEEK_SET);
	std::fread(string_table, 1, string_table_size, _f);

	std::fseek(_f, 16, SEEK_SET);
	char binary_entry[16];
	for (int i = 0; i < num_entries; i++) {
		std::fread(binary_entry, 1, 16, _f);
		int fname_offset = READ_LE_UINT32(binary_entry);
		int start = READ_LE_UINT32(binary_entry + 4);
		int size = READ_LE_UINT32(binary_entry + 8);

		std::string fname = string_table + fname_offset;
		std::transform(fname.begin(), fname.end(), fname.begin(), tolower);

		_fileMap.insert(std::make_pair(fname, LabEntry(start, size)));
		_fileMap.size();
	}

	delete [] string_table;
	return true;
}

bool Lab::fileExists(const char *filename) const {
	return findFilename(filename) != _fileMap.end();
}

Block *Lab::getFileBlock(const char *filename) const {
	FileMapType::const_iterator i = findFilename(filename);
	if (i == _fileMap.end())
		return NULL;

	std::fseek(_f, i->second.offset, SEEK_SET);

	// The sound decoder reads up to two bytes past the end of data
	// (but shouldn't actually use those bytes).  So allocate two extra bytes
	// to be safe against crashes.
	char *data = new char[i->second.len + 2];
	std::fread(data, 1, i->second.len, _f);
	data[i->second.len] = '\0';	// For valgrind cleanness
	data[i->second.len + 1] = '\0';
	return new Block(data, i->second.len);
}

std::FILE *Lab::openNewStream(const char *filename) const {
	FileMapType::const_iterator i = findFilename(filename);
	if (i == _fileMap.end())
		return NULL;

	FILE *file = std::fopen(_labFileName.c_str(), "rb");
	assert(file);
	std::fseek(file, i->second.offset, SEEK_SET);

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
	if (_f != NULL)
		std::fclose(_f);
	_f = NULL;
	_fileMap.clear();
}
