/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef SCOOBY_FILE_H
#define SCOOBY_FILE_H

#include "common/file.h"
#include "common/stream.h"

namespace Scooby {

class ScoobyEngine;

class File : public Common::SeekableReadStream, public Common::ReadStreamEndian {
public:
	explicit File(ScoobyEngine *vm);
	~File() override;

	uint32 decompressBytes(uint32 offset, byte *dataPtr, uint32 dataSize);

	bool eos() const override;

	uint32 read(void *dataPtr, uint32 dataSize) override;

	int64 pos() const override;

	int64 size() const override;

	bool seek(int64 offset, int whence) override;

	bool offsetFromStart(int32 offset);
private:
	ScoobyEngine *_vm;
	Common::File _fd;
public:

};


} // End of namespace Scooby

#endif //SCOOBY_FILE_H
