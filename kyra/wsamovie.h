/* ScummVM - Kyrandia Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
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

#ifndef MOVIES_H
#define MOVIES_H

#include "resource.h"

namespace Kyra {

	// a generic movie
	class Movie {
	
	public:
	
		virtual ~Movie() {}
		
		virtual const uint8* loadFrame(uint16 frame, uint16* width = 0, uint16* height = 0) = 0;
		virtual uint16 countFrames(void) { return _frameCount; }
		
		virtual bool hasPalette(void) { return (_ownPalette != 0); }
		virtual Palette* palette(void) { return _ownPalette; }
	
	protected:
		uint16 _frameCount;
		Palette* _ownPalette;
	};
	
	// movie format for Kyrandia 1
	// there is also a new WSA Format for Kyrandia 2
	// which i will implement in future
	class WSAMovieV1 : public Movie {
	
	public:
	
		WSAMovieV1(uint8* data, uint32 size);
		~WSAMovieV1();
		
		const uint8* loadFrame(uint16 frame, uint16* width, uint16* height);
	protected:
	
		uint8* _buffer;
	
#pragma START_PACK_STRUCTS	
		struct WSAHeader {
			uint16 _numFrames; // All right
			uint16 _width; // should be right
			uint16 _height; // should be right
			uint8 _xPos; // could be wrong
			uint8 _yPos; // could be wrong
			uint16 _delta; // could be wrong
			uint16 _type; // should be right
		} GCC_PACK _wsaHeader;
#pragma END_PACK_STRUCTS

		uint32* _offsetTable;
		
		uint8* _currentFrame;
		uint16 _prefetchedFrame;
	};
} // end of namespace Kyra

#endif

