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

#ifndef MEDIASTATION_DATAFILE_H
#define MEDIASTATION_DATAFILE_H

#include "common/file.h"
#include "common/stream.h"
#include "common/path.h"

namespace MediaStation {

// A Media Station datafile consists of one or more RIFF-style "subfiles". Aside
// from some oddness at the start of the subfile, each subfile is basically
// standard sequence of chunks inside a LIST chunk, like you'd see in any RIFF
// file. These chunks have special IDs:
//  - igod: Indicates a chunk that contains metadata about asset(s) in metadata sections.
//  - a000, where 000 is a string that represents a 3-digit hexadecimal number.
//          Indicates a chunk that contains actor data (sounds and bitmaps).

class Chunk : public Common::SeekableReadStream {
public:
	Chunk() = default;
	Chunk(Common::SeekableReadStream *stream);

	uint32 bytesRemaining();

	uint32 _id = 0;
	uint32 _length = 0;

	// ReadStream implementation
	virtual bool eos() const { return _parentStream->eos(); };
	virtual bool err() const {return _parentStream->err(); };
	virtual void clearErr() { _parentStream->clearErr(); };
	virtual uint32 read(void *dataPtr, uint32 dataSize);
	virtual int64 pos() const { return _parentStream->pos(); };
	virtual int64 size() const { return _parentStream->size(); };
	virtual bool skip(uint32 offset) { return seek(offset, SEEK_CUR); };
	virtual bool seek(int64 offset, int whence = SEEK_SET);

private:
	Common::SeekableReadStream *_parentStream = nullptr;
	uint32 _dataStartOffset = 0;
	uint32 _dataEndOffset = 0;
};

class Subfile {
public:
	Subfile() = default;
	Subfile(Common::SeekableReadStream *stream);

	Chunk nextChunk();
	bool atEnd();

	Chunk _currentChunk;
	uint32 _rate;

private:
	Common::SeekableReadStream *_stream = nullptr;
	Chunk _rootChunk;
};

class Datafile : public Common::File {
public:
	Datafile(const Common::Path &path);

	Subfile getNextSubfile();
};

} // End of namespace MediaStation

#endif