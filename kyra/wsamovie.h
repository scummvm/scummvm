/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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

#ifndef WSAMOVIES_H
#define WSAMOVIES_H

#include "kyra/resource.h"

namespace Kyra {

// a generic movie
class Movie {
	
public:
	
	virtual ~Movie() { _transparency = -1; _ownPalette = 0; _frameCount = 0; }
		
	virtual void renderFrame(uint8* plane, uint16 planepitch, uint16 planeheight, uint16 frame) = 0;
	virtual const uint8* loadFrame(uint16 frame, uint16* width = 0, uint16* height = 0) = 0;
	virtual uint16 countFrames(void) { return _frameCount; }
		
	// could be deleted(not imdiantly maybe it's needed sometime)
	virtual void transparency(int16 color) { _transparency = color; }
	virtual void position(uint16 x, uint16 y) = 0;
		
	virtual bool hasPalette(void) { return (_ownPalette != 0); }
	virtual Palette* palette(void) { return _ownPalette; }
	
	virtual bool looping(void) { return false; }
	virtual uint32 frameChange(void) { return 100; }		
	virtual void setImageBackground(uint8* plane, uint16 planepitch, uint16 height) {};
	
protected:
	int16 _transparency;
	uint16 _frameCount;
	Palette* _ownPalette;
};

// movie format for Kyrandia 1
class WSAMovieV1 : public Movie {
	
public:
	
	WSAMovieV1(uint8* data, uint32 size, uint8 gameid);
	~WSAMovieV1();
		
	void renderFrame(uint8* plane, uint16 planepitch, uint16 planeheight, uint16 frame);
	const uint8* loadFrame(uint16 frame, uint16* width, uint16* height);
	void setImageBackground(uint8* plane, uint16 planepitch, uint16 height);
	
	void position(uint16 x, uint16 y) { _wsaHeader._xPos = x; _wsaHeader._yPos = y; }
protected:
	
	uint8* _buffer;
	
#pragma START_PACK_STRUCTS	
	struct WSAHeader {
		uint16 _numFrames; // All right
		uint16 _width; // All right
		uint16 _height; // All right
		uint8 _xPos; // is wrong
		uint8 _yPos; // is wrong
		uint16 _delta; // is wrong
		uint16 _type; // is wrong
	} GCC_PACK _wsaHeader;
#pragma END_PACK_STRUCTS

	uint32* _offsetTable;
	
	uint8* _currentFrame;
	uint16 _prefetchedFrame;
	
	uint8* _background; // only a pointer to the screen
	uint16 _backWidth, _backHeight;
};
	
// movie format for Kyrandia 2+
class WSAMovieV2 : public Movie {
	
public:
	WSAMovieV2(uint8* data, uint32 size);
	~WSAMovieV2();
	
	void renderFrame(uint8* plane, uint16 planepitch, uint16 planeheight, uint16 frame);
	const uint8* loadFrame(uint16 frame, uint16* width, uint16* height);
	void setImageBackground(uint8* plane, uint16 planepitch, uint16 height);
	
	void position(uint16 x, uint16 y) { _wsaHeader._xPos = x; _wsaHeader._yPos = y; }
	bool looping(void) { return _looping; }
protected:
	
	uint8* _buffer;
		
	struct WSAHeader {
		uint16 _numFrames; // All right
		uint16 _width; // should be right
		uint16 _height; // should be right
		uint16 _xPos; // could be wrong
		uint16 _yPos; // could be wrong
		uint16 _delta; // could be wrong
		uint16 _type; // should be right
	} GCC_PACK _wsaHeader;
	
	uint32* _offsetTable;
	
	uint8* _currentFrame;
	uint16 _prefetchedFrame;
	
	uint8* _background; // only a pointer to the screen
	uint16 _backWidth, _backHeight;
	
	bool _looping;
};
} // end of namespace Kyra

#endif

