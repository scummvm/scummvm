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

#ifndef CHEWY_FILE_H
#define CHEWY_FILE_H

#include "common/algorithm.h"
#include "common/file.h"
#include "common/textconsole.h"

namespace Chewy {

using Common::Stream;

class File {
public:
	/**
	 * Opens up a file, returning it if successful,
	 * or nullptr if not
	 */
	static Common::File *open(const char *name);

	/**
	 * Reads an array of uint16 values
	 */
	static bool readArray(Common::SeekableReadStream *src,
		uint16 *arr, size_t size);
};


inline Stream *chewy_fopen(const char *fname, const char *mode) {
	assert(!strcmp(mode, "rb"));
	return File::open(fname);
}

inline void chewy_fclose(Stream *&stream) {
	delete stream;
	stream = nullptr;
}

} // namespace Chewy

#endif
