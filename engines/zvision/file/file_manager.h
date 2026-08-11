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

#include "common/file.h"
#include "common/path.h"
#include "zvision/zvision.h"

#ifndef ZVISION_FILE_MANAGER
#define ZVISION_FILE_MANAGER

namespace ZVision {

class FileManager {
public:
	FileManager(ZVision *engine);
	~FileManager() {};

	bool loadZix(const Common::Path &zixPath, const Common::FSNode &gameDataDir);
	Common::File *open(const Common::Path &fileName, bool allowSrc=true);	// Wrapper to automatically handle loading of files which may be empty & have an alternate .src file
	bool exists(Common::Path filePath, bool allowSrc=true);	// Wrapper to automatically handle checking existence of files which may be empty & have an alternate .src file

private:
	Common::Path srcPath(Common::Path filePath);
};

} // End of namespace ZVision

#endif
