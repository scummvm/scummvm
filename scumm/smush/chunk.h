/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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

#include "config.h"

/*! 	@brief Interface for Chunk handling

	This class is an interface for reading from a Chunk.

	\todo handle big endian system.
*/
class Chunk {
public:
	enum seek_type { seek_start, seek_end, seek_cur };
	virtual ~Chunk() {};
	typedef uint32 type;			//!< type of a Chunk (i.e. The first 4byte field of the Chunk structure).
	/*!	@brief convert a type to a string
		
		Utility function that convert a type to a string.
		
		@param t the type to convert to a string
		
		@return the converted string
	*/
	static const char * ChunkString(type t);

	virtual type getType() const = 0;	//!< return the type of the Chunk
	virtual uint32 getSize() const = 0;	//!< return the size of the Chunk
	virtual Chunk * subBlock() = 0; //!< extract a subChunk from the current read position
	virtual bool eof() const = 0;	//!< is the Chunk completely read ?
	virtual uint32 tell() const = 0;	//!< get the Chunk current read position
	virtual bool seek(int32 delta, seek_type dir = seek_cur) = 0;	//!< move the current read position inside the Chunk
	virtual bool read(void * buffer, uint32 size) = 0;		//!< read some data for the current read position
	virtual int8 getChar() = 0;							//!< extract the character at the current read position
	virtual byte getByte() = 0;					//!< extract the byte at the current read position
	virtual int16 getShort() = 0;						//!< extract the short at the current read position
	virtual uint16 getWord() = 0;					//!< extract the word at the current read position
	virtual uint32 getDword()= 0;					//!< extract the dword at the current read position
};

class FilePtr;

/*! 	@brief file based ::Chunk

	This class is an implementation of ::Chunk that handles file.

*/
class FileChunk : public Chunk {
private:
	FilePtr * _data;
	type _type;
	uint32 _size;
	uint32 _offset;
	uint32 _curPos;
protected:
	FileChunk();
public:
	FileChunk(const char * fname, const char * directory);
	virtual ~FileChunk();
	type getType() const;
	uint32 getSize() const;
	Chunk * subBlock();
	bool eof() const;
	uint32 tell() const;
	bool seek(int32 delta, seek_type dir = seek_cur);
	bool read(void * buffer, uint32 size);
	int8 getChar();
	byte getByte();
	short getShort();
	uint16 getWord();
	uint32 getDword();
};

/*! 	@brief memory based ::Chunk

	This class is an implementation of ::Chunk that handles a memory buffer.
*/
class ContChunk : public Chunk {
private:
	byte *_data;
	Chunk::type _type;
	uint32 _size;
	uint32 _curPos;
public:
	ContChunk(byte *data);
	Chunk::type getType() const;
	uint32 getSize() const;
	Chunk *subBlock();
	bool eof() const;
	uint32 tell() const;
	bool seek(int32 delta, seek_type dir = seek_cur);
	bool read(void *buffer, uint32 size);
	int8 getChar();
	byte getByte();
	int16 getShort();
	uint16 getWord();
	uint32 getDword();
};

#endif
