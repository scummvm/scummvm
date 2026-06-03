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

#ifndef FREESCAPE_ZX_TAPE_H
#define FREESCAPE_ZX_TAPE_H

#include "common/array.h"
#include "common/path.h"

namespace Common {
class Archive;
class FSNode;
class SeekableReadStream;
}

struct ADGameDescription;

namespace Freescape {

struct ZxTapeFile {
	Common::Path name;
	Common::Array<byte> data;
};

typedef Common::Array<ZxTapeFile> ZxTapeFileList;

bool extractZxSpectrumTapeFiles(Common::SeekableReadStream &stream, const char *prefix, ZxTapeFileList &files);
bool matchZxSpectrumTapeFiles(const ZxTapeFileList &files, const ADGameDescription &desc, uint md5Bytes = 5000);
Common::Archive *makeZxSpectrumTapeArchive(const ADGameDescription &desc, const Common::Path &gamePath);

} // End of namespace Freescape

#endif
