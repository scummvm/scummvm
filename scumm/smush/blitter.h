/* ScummVM - Scumm int32erpreter
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

#ifndef BLITTER_H
#define BLITTER_H

#include "config.h"

#ifdef DEBUG
# ifndef NO_DEBUG_CLIPPER
#  define DEBUG_CLIPPER
# endif
#else
# ifdef DEBUG_CLIPPER
#  error DEBUG_CLIPPER defined without DEBUG
# endif
#endif

#include "common/rect.h"

using ScummVM::Point;
using ScummVM::Rect;

class Chunk;
/*! 	@brief class for handling blitting on a frame buffer

	This class allows to perform secure blitting to a frame buffer in several ways.
	This means that clipping is performed, so that only the part that you want to modify can be modified.
*/
class Blitter {
private:
	byte * _ptr;	//!< This is the pointer to the start of the frame buffer
	byte * _offset;	//!< This is the current pointer in the frame buffer
	Point _clip;		//!<  This is the size of the frame buffer (width/height)
	Rect _src; 		//!< This is the size and position of the destination rectangle
	Point _cur; 		//!< This is the current position in the destination rectangle
	bool _outside;	//!< flag that is set to \c true when the blitter reach the end of the destination rectangle
#ifdef DEBUG_CLIPPER
	int32 _clipped;
	int32 _clippedBlock;
#endif
public:
	/*!	@brief constructor

		@param buffer the frame buffer to blit to
		@param dstsize the size of the frame buffer
		@param src the rectangle to blit to
	*/
	Blitter(byte * buffer, const Point & dstsize, const Rect & src);
	virtual ~Blitter();
	void blit(byte *, uint32); //!< This method allows to blit directly some data from a buffer
	void blit(Chunk &, uint32); //!< This method allows to blit directly some data from a Chunk
	void put(byte); //!< This method allows to blit one byte
	void put(byte, uint32); //!< This method allows to blit one byte several times
	void advance(int32 = 1, int32 = 0); //!< This method allows to advance the current position in the blitter
	void advanceBlock(int32 = 1, int32 = 0); //!< This method allows to advance the current position in the blitter in terms of blocks
	void putBlock(uint32); //!< This method allows to blit one block from an int32 value repeated 4 time
	void putBlock(Chunk &); //!< This method allows to blit one block directly read from a Chunk
	void putBlock(byte *); //!< This method allows to blit one block directly from a buffer
	void putBlock(uint32, uint32, uint32, uint32); //!< This method allows to blit one block from a 4 int32 value
	void blockCopy(int32); //!< This method allows to copy one block from another separated by the given offset
};

#endif
