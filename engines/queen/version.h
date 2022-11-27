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

#ifndef QUEEN_VERSION_H
#define QUEEN_VERSION_H

#include "common/language.h"
#include "common/platform.h"

namespace Common {
class File;
}

namespace Queen {

struct DetectedGameVersion {
	Common::Platform platform;
	Common::Language language;
	uint8 features;
	uint8 compression;
	char str[6];
	uint8 queenTblVersion;
	uint32 queenTblOffset;
};

struct RetailGameVersion {
	char str[6];
	uint8 queenTblVersion;
	uint32 queenTblOffset;
	uint32 dataFileSize;
};

//! detect game version
bool detectVersion(DetectedGameVersion *ver, Common::File *f);

//! detect game version based on queen.1 datafile size
const RetailGameVersion *detectGameVersionFromSize(uint32 size);

} // End of namespace Queen

#endif
