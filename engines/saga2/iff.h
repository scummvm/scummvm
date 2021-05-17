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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_IFF_H
#define SAGA2_IFF_H

#include "saga2/errors.h"

namespace Saga2 {

enum iffErrCodes {
	errNotIFF = 0x100,                      // not an iff form
	errMangledForm,                         // data in form is mangled
	errEndOfFile,                           // unexpected EOF reached
};

//  Right now I don't care about clipboard or memory reading...

typedef uint32      ChunkID;

#define MakeID(a,b,c,d) (((ChunkID)d<<24L)|((ChunkID)c<<16L)|(b<<8L)|a)

const ChunkID       ID_FORM = MakeID('F', 'O', 'R', 'M'),
                    ID_NAME = MakeID('N', 'A', 'M', 'E'),
                    ID_AUTH = MakeID('A', 'U', 'T', 'H'),
                    ID_COPYRIGHT = MakeID('(', 'C', ')', ' ');

struct chunkInfo {
	ChunkID         id;                     // id of this chunk
	int32           size,                   // size of chunk
	                pos;                    // filepos within chunk
	int32           patch;                  // file position to patch
};

//  Base class for both reading and writing handles

class iffHandle {
protected:
	char            *name;                  // name of file
	FILE            *fileHandle;            // handle to raw file
	uint16          flags;
	errorCode       lastErr;

	chunkInfo       form,                   // info on current FORM
	                chunk;                  // info on current chunk

	iffHandle(void);

	void swapLong(int32 &l);                 // put bytes in moto order

public:
	errorCode lastError(void) {              // returns last error code
		return lastErr;
	}

	void setError(errorCode e) {         // sets error code
		lastErr = e;
	}

	int32 chunkLength(void) {                // length of this chunk
		return chunk.size;
	}

	int32 chunkPos(void) {                   // position of chunk
		return chunk.pos;
	}

	int32 formLength(void) {                 // length of this form
		return form.size;
	}

	int32 formPos(void) {                    // position of form
		return form.pos;
	}

	char *getFileName(void) {
		return name;
	}
};

enum iffFlags {
	iffStreamIO = (1 << 0),
};

//  IFF reading handle

class iffReadHandle : public iffHandle {
//	ChunkID          *stopList;

	bool readChunk(void);
	bool skipChunk(void);
public:
	iffReadHandle(char *filename, ChunkID &formName);
	virtual ~iffReadHandle(void);

	bool nextChunk(ChunkID &id);
	int32 read(void *buffer, int32 length);

//	bool seekChunk( ChunkID );
//	void stopChunks( ChunkID *stopList );
};

//  IFF writing handle

class iffWriteHandle : public iffHandle {
public:
	iffWriteHandle(char *filename, ChunkID formName);
	virtual ~iffWriteHandle(void);

	bool pushChunk(ChunkID id, int32 length = -1);
	bool popChunk(void);

	//  Write bytes
	int32 write(void *buffer, int32 length);

	//  Write a whole chunk
	int32 writeChunk(ChunkID id, void *buffer, int32 length);
};

void swapLong(int32 &l);
void swapWord(int16 &w);

} // end of namespace Saga2

#endif
