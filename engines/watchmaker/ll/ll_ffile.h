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

#ifndef WATCHMAKER_LL_FFILE_H
#define WATCHMAKER_LL_FFILE_H

#include "common/ptr.h"
#include "common/stream.h"

namespace Watchmaker {

struct FileEntry;
class FastFile {
	const char *_path = nullptr;
	int _totalSize = 0;
public:
	int _numFiles = 0;
	FileEntry *_files = nullptr; // TODO: This could just be a Common::Array

	FastFile(const char *path);
	~FastFile();

	Common::SharedPtr<Common::SeekableReadStream> resolve(const char *filename);
};


} // End of namespace Watchmaker

#endif // WATCHMAKER_LL_FFILE_H
