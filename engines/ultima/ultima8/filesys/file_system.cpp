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

#include "ultima/ultima8/filesys/file_system.h"
#include "ultima/ultima8/ultima8.h"
#include "common/savefile.h"

namespace Ultima {
namespace Ultima8 {

using Std::string;

FileSystem *FileSystem::_fileSystem = nullptr;

FileSystem::FileSystem() {
	debugN(MM_INFO, "Creating FileSystem...\n");

	_fileSystem = this;
}

FileSystem::~FileSystem() {
	debugN(MM_INFO, "Destroying FileSystem...\n");

	_fileSystem = nullptr;
}


// Open a streaming file as readable. Streamed (0 on failure)
Common::SeekableReadStream *FileSystem::ReadFile(const string &vfn) {
	Common::SeekableReadStream *readStream;
	if (!rawOpen(readStream, vfn)) {
		// Some games have some files in a "data" subdir.
		string altpath = string::format("data/%s", vfn.c_str());
		if (!rawOpen(readStream, altpath)) {
			return nullptr;
		}
	}

	return readStream;
}

// Open a streaming file as writeable. Streamed (0 on failure)
Common::WriteStream *FileSystem::WriteFile(const string &vfn) {
	string filename = vfn;
	Common::WriteStream *writeStream;

	if (!rawOpen(writeStream, filename))
		return nullptr;

	return writeStream;
}

bool FileSystem::rawOpen(Common::SeekableReadStream *&in, const string &fname) {
	string name = fname;
	Common::File *f;

	int uppercasecount = 0;
	f = new Common::File();
	do {
		if (f->open(name)) {
			in = f;
			return true;
		}
	} while (base_to_uppercase(name, ++uppercasecount));

	// file not found
	delete f;
	return false;
}


bool FileSystem::rawOpen(Common::WriteStream *&out,  const string &fname) {
	string name = fname;

	return false;
}

/*
 *  Convert just the last 'count' parts of a filename to uppercase.
 *  returns false if there are less than 'count' parts
 */

bool FileSystem::base_to_uppercase(string &str, int count) {
	if (count <= 0) return true;

	int todo = count;
	// Go backwards.
	string::reverse_iterator X;
	for (X = str.rbegin(); X != str.rend(); ++X) {
		// Stop at separator.
		if (*X == '/' || *X == '\\' || *X == ':')
			todo--;
		if (todo <= 0)
			break;

		*X = static_cast<char>(toupper(*X));
	}
	if (X == str.rend())
		todo--; // start of pathname counts as separator too

	// false if it didn't reach 'count' parts
	return (todo <= 0);
}

} // End of namespace Ultima8
} // End of namespace Ultima
