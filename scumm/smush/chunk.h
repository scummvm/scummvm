/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef CHUNK_H
#define CHUNK_H

#include "common/scummsys.h"

namespace Scumm {

class Chunk {
public:
	virtual ~Chunk() {};
	enum seek_type { seek_start, seek_end, seek_cur };
	typedef uint32 type;
	static const char *ChunkString(type t);
	virtual type getType() const = 0;
	virtual uint32 getSize() const = 0;
	virtual Chunk *subBlock() = 0;
	virtual bool eof() const = 0;
	virtual uint32 tell() const = 0;
	virtual bool seek(int32 delta, seek_type dir = seek_cur) = 0;
	virtual bool read(void *buffer, uint32 size) = 0;
	virtual int8 getChar() = 0;
	virtual byte getByte() = 0;
	virtual int16 getShort() = 0;
	virtual uint16 getWord() = 0;
	virtual uint32 getDword()= 0;
};

class FilePtr;

// Common functionality for concrete chunks (FileChunk, MemoryChunk)
class BaseChunk : public Chunk {
protected:
	Chunk::type _type;
	uint32 _size;
	uint32 _curPos;

	BaseChunk();

public:
	Chunk::type getType() const;
	uint32 getSize() const;
	bool eof() const;
	uint32 tell() const;
	bool seek(int32 delta, seek_type dir = seek_cur);
};

class FileChunk : public BaseChunk {
private:
	FilePtr *_data;
	uint32 _offset;

protected:
	FileChunk();

public:
	FileChunk(const char *fname, const char *directory);
	virtual ~FileChunk();
	Chunk *subBlock();
	bool read(void *buffer, uint32 size);
	int8 getChar();
	byte getByte();
	short getShort();
	uint16 getWord();
	uint32 getDword();
};

class MemoryChunk : public BaseChunk {
private:
	byte *_data;

public:
	MemoryChunk(byte *data);
	Chunk *subBlock();
	bool read(void *buffer, uint32 size);
	int8 getChar();
	byte getByte();
	int16 getShort();
	uint16 getWord();
	uint32 getDword();
};

} // End of namespace Scumm

#endif
