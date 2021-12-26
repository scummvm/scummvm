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

#ifndef ULTIMA8_FILESYS_FILESYSTEM_H
#define ULTIMA8_FILESYS_FILESYSTEM_H

#include "ultima/shared/std/string.h"
#include "common/file.h"

namespace Ultima {
namespace Ultima8 {

class FileSystem {
public:
	FileSystem();
	~FileSystem();

	static FileSystem *get_instance() {
		return _fileSystem;
	}

	//! Open a file as readable. Streamed.
	//! \param vfn the (virtual) filename
	//! \return nullptr on failure
	Common::SeekableReadStream *ReadFile(const Std::string &vfn);

	//! Open a file as writable. Streamed.
	//! \param vfn the (virtual) filename
	//! \return a new writestream, or nullptr on failure
	Common::WriteStream *WriteFile(const Std::string &vfn);

private:
	static bool base_to_uppercase(Std::string &str, int count);

	static FileSystem *_fileSystem;

	/**
	 *	Open a file for reading,
	 *	Output: false if couldn't open.
	 */
	bool rawOpen(Common::SeekableReadStream *&in, const Std::string &fname);

	/**
	 *	Open a file for writing,
	 *	Output: false if couldn't open.
	 */
	bool rawOpen(Common::WriteStream *&out, const Std::string &fname);
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
