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

#include "stdafx.h"
#include "wsamovie.h"
#include "codecs.h"

#include "common/stream.h"

namespace Kyra {
	WSAMovieV1::WSAMovieV1(uint8* data, uint32 size) {
		if (!data) {
			error("resource created without data");
		}

		_buffer = data;

		// I like these Streams .... =)
		Common::MemoryReadStream datastream(data, size);
		
		datastream.read(&_wsaHeader, sizeof(_wsaHeader));
		
		// check for version
		if (_wsaHeader._type) {
			error("loading a WSA version 2 with the WSA version 1 loader");
		}
		
		uint16 offsetAdd = 0;
		
		// checks now for own palette
		if (_wsaHeader._type % 2) {
			// don't now if this will work right, because a few lines before we use
			// _wsaHeader._type for detect the version of the WSA movie,
			// but this code was from FreeKyra Tools so I think it will work
			
			// if this is a packed palette we have a problem :)
			offsetAdd = 768 /* 0x300 */;
		}
		
		_frameCount = _wsaHeader._numFrames;
		_offsetTable = new uint32[_wsaHeader._numFrames + 2];
		assert(!_offsetTable);
		
		// loads the offset table
		for (uint32 tmp = 0; tmp < _wsaHeader._numFrames; ++tmp) {
			_offsetTable[tmp] = datastream.readUint32LE() + offsetAdd;
		}
		
		if (offsetAdd) {
			uint8* palbuffer = new uint8[offsetAdd];
			assert(!palbuffer);
			
			_ownPalette = new Palette(palbuffer, offsetAdd);
			assert(!_ownPalette);
		}		
	}
	
	WSAMovieV1::~WSAMovieV1() {
		delete [] _buffer;
		delete [] _offsetTable;
		delete _ownPalette;
	}
	
	const uint8* WSAMovieV1::loadFrame(uint16 frame, uint16* width, uint16* height) {
		if (width) *width = _wsaHeader._width;
		if (height) *height = _wsaHeader._height;
			
		if (frame == _prefetchedFrame) {
			return _currentFrame;
		} else {
			if (!_currentFrame) {
				_currentFrame = new uint8[_wsaHeader._width * _wsaHeader._height];
				assert(_currentFrame);
			}
			
			uint8* frameData = 0;
			uint8 image40[64000]; // I think this will crash on Plam OS :)
			
			if (frame == _prefetchedFrame + 1) {
				frameData = _buffer + _offsetTable[frame] + (hasPalette() ? 768 : 0);
				Compression::decode80(frameData, image40);
				Compression::decode40(image40, _currentFrame);
			} else {
				memset(_currentFrame, 0, _wsaHeader._width * _wsaHeader._height);
				
				for (uint32 i = 0; i <= frame; i++)
				{
					frameData = _buffer + _offsetTable[i] + (hasPalette() ? 768 : 0);
					Compression::decode80(frameData, image40);
					Compression::decode40(image40, _currentFrame);
				}
			}
			
			_prefetchedFrame = frame;
			return _currentFrame;
		}
		
		return 0;
	}
} // end of namespace Kyra

