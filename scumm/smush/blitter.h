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

#ifndef __BLITTER_H_
#define __BLITTER_H_

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

#include "rect.h"

class Chunck;
/*! 	@brief class for handling blitting on a frame buffer

	This class allows to perform secure blitting to a frame buffer in several ways.
	This means that clipping is performed, so that only the part that you want to modify can be modified.
*/
class Blitter {
private:
	char * _ptr;	//!< This is the pointer to the start of the frame buffer
	char * _offset;	//!< This is the current pointer in the frame buffer
	Point _clip;		//!<  This is the size of the frame buffer (width/height)
	Rect _src; 		//!< This is the size and position of the destination rectangle
	Point _cur; 		//!< This is the current position in the destination rectangle
	bool _outside;	//!< flag that is set to \c true when the blitter reach the end of the destination rectangle
#ifdef DEBUG_CLIPPER
	int _clipped;
	int _clippedBlock;
#endif
public:
	/*!	@brief constructor

		@param buffer the frame buffer to blit to
		@param dstsize the size of the frame buffer
		@param src the rectangle to blit to
	*/
	Blitter(char * buffer, const Point & dstsize, const Rect & src);
	virtual ~Blitter();
	void blit(char *, unsigned int); //!< This method allows to blit directly some data from a buffer
	void blit(Chunck &, unsigned int); //!< This method allows to blit directly some data from a chunck
	void put(char); //!< This method allows to blit one byte
	void put(char, unsigned int); //!< This method allows to blit one byte several times
	void advance(int = 1, int = 0); //!< This method allows to advance the current position in the blitter
	void advanceBlock(int = 1, int = 0); //!< This method allows to advance the current position in the blitter in terms of blocks
	void putBlock(unsigned int); //!< This method allows to blit one block from an int value repeated 4 time
	void putBlock(Chunck &); //!< This method allows to blit one block directly read from a chunck
	void putBlock(unsigned char *); //!< This method allows to blit one block directly from a buffer
	void putBlock(unsigned int, unsigned int, unsigned int, unsigned int); //!< This method allows to blit one block from a 4 int value
	void blockCopy(int); //!< This method allows to copy one block from another separated by the given offset
};

#endif
