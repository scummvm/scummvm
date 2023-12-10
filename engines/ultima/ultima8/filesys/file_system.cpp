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
	_fileSystem = this;
}

FileSystem::~FileSystem() {
	_fileSystem = nullptr;
}


// Open a streaming file as readable. Streamed (0 on failure)
Common::SeekableReadStream *FileSystem::ReadFile(const string &vfn) {
	Common::File *f = new Common::File();
	Common::Path path(vfn);
	if (f->open(path))
		return f;

	// Some games have some files in a "data" subdir.
	Common::Path altpath = Common::Path("data").join(path);
	if (f->open(altpath))
		return f;

	delete f;
	return nullptr;
}

} // End of namespace Ultima8
} // End of namespace Ultima
