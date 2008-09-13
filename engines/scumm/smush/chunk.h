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
 * $URL$
 * $Id$
 *
 */

#ifndef SCUMM_SMUSH_CHUNK_H
#define SCUMM_SMUSH_CHUNK_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/stream.h"

namespace Scumm {

class BaseScummFile;

class Chunk : public Common::SeekableReadStream {
public:
	typedef uint32 type;

	virtual type getType() const = 0;
	virtual Chunk *subBlock() = 0;
	virtual void reseek() = 0;
};

// Common functionality for concrete chunks (FileChunk, MemoryChunk)
class BaseChunk : public Chunk {
protected:
	Chunk::type _type;
	uint32 _size;
	uint32 _curPos;
	Common::String _name;

	BaseChunk();

public:
	Chunk::type getType() const;
	int32 size() const;
	bool eos() const;
	int32 pos() const;
	bool seek(int32 delta, int dir);
};

class FileChunk : public BaseChunk {
private:
	BaseScummFile *_data;
	bool _deleteData;
	uint32 _offset;

	FileChunk(BaseScummFile *data, int offset);
public:
	FileChunk(const Common::String &name, int offset = 0);
	virtual ~FileChunk();
	Chunk *subBlock();
	void reseek();
	uint32 read(void *buffer, uint32 size);
};

class MemoryChunk : public BaseChunk {
private:
	byte *_data;

public:
	MemoryChunk(byte *data);
	Chunk *subBlock();
	void reseek();
	uint32 read(void *buffer, uint32 size);
};

} // End of namespace Scumm

#endif
