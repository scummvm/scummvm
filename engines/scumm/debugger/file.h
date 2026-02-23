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

#ifndef SCUMM_EDITOR_FILE_H
#define SCUMM_EDITOR_FILE_H

#include "common/noncopyable.h"
#include "common/path.h"
#include "common/stream.h"

namespace Scumm {

namespace Editor {

class File : public Common::NonCopyable {
private:
	Common::Path _path;
	byte _encByte;
	Common::SeekableReadStream *_stream;

public:
	File();
	~File();

	bool open(const Common::Path &path, byte encByte);
	void close();

	const Common::Path &getPath() const;

	int64 pos() const;
	int64 size() const;
	bool seek(int64 offs, int whence = SEEK_SET);
	uint32 read(void *dataPtr, uint32 dataSize);

	byte readByte();
	uint16 readUint16LE();
	uint32 readUint32LE();
	uint16 readUint16BE();
	uint32 readUint32BE();
};

} // End of namespace Editor

} // End of namespace Scumm

#endif
