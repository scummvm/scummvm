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

#ifndef __CHUNCK_H_
#define __CHUNCK_H_

#include "config.h"

/*! 	@brief Interface for chunck handling

	This class is an interface for reading from a chunck.

	\todo handle big endian system.
*/
class Chunck {
public:
	enum seek_type { seek_start, seek_end, seek_cur };
	virtual ~Chunck() {};
	typedef unsigned int type;			//!< type of a chunck (i.e. The first 4byte field of the chunck structure).
	/*!	@brief convert a type to a string
		
		Utility function that convert a type to a string.
		
		@param t the type to convert to a string
		
		@return the converted string
	*/
	static const char * ChunckString(type t);

	virtual type getType() const = 0;	//!< return the type of the chunck
	virtual unsigned int getSize() const = 0;	//!< return the size of the chunck
	virtual Chunck * subBlock() = 0; //!< extract a subchunck from the current read position
	virtual bool eof() const = 0;	//!< is the chunck completely read ?
	virtual unsigned int tell() const = 0;	//!< get the chunck current read position
	virtual bool seek(int delta, seek_type dir = seek_cur) = 0;	//!< move the current read position inside the chunck
	virtual bool read(void * buffer, unsigned int size) = 0;		//!< read some data for the current read position
	virtual char getChar() = 0;							//!< extract the character at the current read position
	virtual unsigned char getByte() = 0;					//!< extract the byte at the current read position
	virtual short getShort() = 0;						//!< extract the short at the current read position
	virtual unsigned short getWord() = 0;					//!< extract the word at the current read position
	virtual unsigned int getDword()= 0;					//!< extract the dword at the current read position
};

class FilePtr;

/*! 	@brief file based ::chunck

	This class is an implementation of ::chunck that handles file.

*/
class FileChunck : public Chunck {
private:
	FilePtr * _data;
	type _type;
	unsigned int _size;
	unsigned int _offset;
	unsigned int _curPos;
protected:
	FileChunck();
public:
	FileChunck(const char * fname);
	virtual ~FileChunck();
	type getType() const;
	unsigned int getSize() const;
	Chunck * subBlock();
	bool eof() const;
	unsigned int tell() const;
	bool seek(int delta, seek_type dir = seek_cur);
	bool read(void * buffer, unsigned int size);
	char getChar();
	unsigned char getByte();
	short getShort();
	unsigned short getWord();
	unsigned int getDword();
};

/*! 	@brief memory based ::chunck

	This class is an implementation of ::chunck that handles a memory buffer.
*/
class ContChunck : public Chunck {
private:
	char * _data;
	Chunck::type _type;
	unsigned int _size;
	unsigned int _curPos;
public:
	ContChunck(char * data);
	Chunck::type getType() const;
	unsigned int getSize() const;
	Chunck * subBlock();
	bool eof() const;
	unsigned int tell() const;
	bool seek(int delta, seek_type dir = seek_cur);
	bool read(void * buffer, unsigned int size);
	char getChar();
	unsigned char getByte();
	short getShort();
	unsigned short getWord();
	unsigned int getDword();
};

#endif
