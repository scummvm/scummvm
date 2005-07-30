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

#include "common/stdafx.h"
#include "kyra/wsamovie.h"
#include "kyra/codecs.h"

#include "common/stream.h"

#ifdef DUMP_FILES
#include <stdio.h>
#endif

namespace Kyra {
WSAMovieV1::WSAMovieV1(uint8* data, uint32 size, uint8 gameid) {
	if (!data) {
		error("resource created without data");
	}

	_background = 0;
	_currentFrame = 0;
	_ownPalette = 0;
	_offsetTable = 0;
	_prefetchedFrame = 0xFFFE;
	_buffer = data;

	// I like these Streams .... =)
	Common::MemoryReadStream datastream(data, size);

	_wsaHeader._numFrames = datastream.readUint16LE();
	_wsaHeader._width = datastream.readUint16LE();
	_wsaHeader._height = datastream.readUint16LE();
	_wsaHeader._xPos = datastream.readByte();
	_wsaHeader._yPos = datastream.readByte();
	_wsaHeader._delta = datastream.readUint16LE();
	_wsaHeader._type = datastream.readUint16LE();

#ifdef DUMP_FILES
	// TODO: make Linux/BSD conform
	FILE* wsaheader = fopen("dumps/wsaheader.txt", "w+");

	if (wsaheader) {
		for (uint32 pos = 0; pos < sizeof(_wsaHeader); ++pos)
			fprintf(wsaheader, "%d pos. byte: %d\n", pos + 1, ((uint8*)&_wsaHeader)[pos]);
		fprintf(wsaheader, "\n");
		for (uint32 pos = 0; pos < sizeof(_wsaHeader) / 2; ++pos)
			fprintf(wsaheader, "%d pos. word: %d\n", pos + 1, ((uint16*)&_wsaHeader)[pos]);
		fprintf(wsaheader, "\n");
		for (uint32 pos = 0; pos < sizeof(_wsaHeader) / 4; ++pos)
			fprintf(wsaheader, "%d pos. dword: %d\n", pos + 1, ((uint32*)&_wsaHeader)[pos]);
	}
	fclose(wsaheader);
#endif

	if (gameid == KYRA1CD) {
		uint16 tmp = _wsaHeader._delta;
		_wsaHeader._delta = _wsaHeader._type;
		_wsaHeader._type = tmp;

		// skip 2 bytes
		datastream.readUint16LE();
	}

	debug("_wsaHeader._numFrames = %d", _wsaHeader._numFrames);
	debug("_wsaHeader._width = %d", _wsaHeader._width);
	debug("_wsaHeader._height = %d", _wsaHeader._height);
	debug("_wsaHeader._xPos = %d", _wsaHeader._xPos);
	debug("_wsaHeader._yPos = %d", _wsaHeader._yPos);
	debug("_wsaHeader._delta = %d", _wsaHeader._delta);
	debug("_wsaHeader._type = %d", _wsaHeader._type);

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

	// last frame seems every time to be a empty one
	_frameCount = _wsaHeader._numFrames - 1;
	_offsetTable = new uint32[_wsaHeader._numFrames + 2];
	assert(_offsetTable);

	// loads the offset table
	for (uint32 tmp = 0; tmp < (uint32)_wsaHeader._numFrames + 2; ++tmp) {
		_offsetTable[tmp] = datastream.readUint32LE() + offsetAdd;
	}

	if (offsetAdd) {
		uint8* palbuffer = new uint8[offsetAdd];
		assert(palbuffer);

		datastream.read(palbuffer, offsetAdd);

		_ownPalette = new Palette(palbuffer, offsetAdd);
		assert(_ownPalette);
	}

	// FIXME: Confirm the default value here?
	// LordHoto: What is the 'default' value? 0?
	_transparency = -1;
}

WSAMovieV1::~WSAMovieV1() {
	delete [] _buffer;
	delete [] _offsetTable;
	delete [] _currentFrame;
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
			memset(_currentFrame, 0, sizeof(uint8) * _wsaHeader._width * _wsaHeader._height);
		}

		if (frame >= _wsaHeader._numFrames)
			return 0;

		uint8* frameData = 0;
		static uint8 image40[64000]; // I think this will crash on Plam OS :)
		memset(image40, 0, ARRAYSIZE(image40));

		if (frame == _prefetchedFrame + 1) {
			frameData = _buffer + _offsetTable[frame];
			Compression::decode80(frameData, image40);
			Compression::decode40(image40, _currentFrame);
		} else {
			if (_background) {
				setImageBackground(_background, _backWidth, _backHeight);
			} else {
				memset(_currentFrame, 0, sizeof(uint8) * _wsaHeader._width * _wsaHeader._height);
			}

			for (uint32 i = 0; i <= frame; ++i)
			{
				frameData = _buffer + _offsetTable[i];
				Compression::decode80(frameData, image40);
				Compression::decode40(image40, _currentFrame);
			}
		}

		_prefetchedFrame = frame;
		return _currentFrame;
	}

	return 0;
}

void WSAMovieV1::renderFrame(uint8* plane, uint16 planepitch, uint16 planeheight, uint16 frame) {
	if (!loadFrame(frame, 0, 0))
		return;

	uint8* src = _currentFrame;
	uint8* dst = &plane[_wsaHeader._yPos * planepitch + _wsaHeader._xPos];
	uint32 copysize = planepitch - _wsaHeader._xPos;

	if (copysize > _wsaHeader._width)
		copysize = _wsaHeader._width;

	if (_transparency == -1) {
		for (uint16 y_ = 0; y_ < _wsaHeader._height && _wsaHeader._yPos + y_ < planeheight; ++y_) {
			memcpy(dst, src, copysize * sizeof(uint8));
			dst += planepitch;
			src += _wsaHeader._width;
		}
	} else {
		for (uint16 yadd = 0; yadd < _wsaHeader._height; ++yadd) {
			for (uint16 xadd = 0; xadd < copysize; ++xadd) {
				if (*src == _transparency) {
					++dst;
					++src;
				} else {
					*dst++ = *src++;
				}
			}

			src += _wsaHeader._width - copysize;
			dst += planepitch - copysize;
		}
	}
}

void WSAMovieV1::setImageBackground(uint8* plane, uint16 planepitch, uint16 height) {
	assert(plane);

	_background = plane;
	_backWidth = planepitch; _backHeight = height;

	if (!_currentFrame) {
		_currentFrame = new uint8[_wsaHeader._width * _wsaHeader._height];
		assert(_currentFrame);
	}

	memset(_currentFrame, 0, sizeof(uint8) * _wsaHeader._width * _wsaHeader._height);

	uint8* src = &plane[_wsaHeader._yPos * planepitch + _wsaHeader._xPos];
	uint8* dst = _currentFrame;
	uint32 copysize = planepitch - _wsaHeader._xPos;

	if (copysize > _wsaHeader._width)
		copysize = _wsaHeader._width;

	// now copy the rect of the plane
	for (uint16 y_ = 0; y_ < _wsaHeader._height && _wsaHeader._yPos + y_ < height; ++y_) {
		memcpy(dst, src, copysize * sizeof(uint8));
		dst += _wsaHeader._width;
		src += planepitch;
	}

	for (uint16 y_ = 0; y_ < _wsaHeader._height && _wsaHeader._yPos + y_ < height; ++y_) {
		for (uint16 x = 0; x < _wsaHeader._width; ++x) {
			_currentFrame[y_ * _wsaHeader._width + x] ^= 0;
		}
	}

	_prefetchedFrame = 0xFFFE;
}

// Kyrandia 2+ Movies
WSAMovieV2::WSAMovieV2(uint8* data, uint32 size) {
	if (!data) {
		error("resource created without data");
	}

	_background = 0;
	_currentFrame = 0;
	_ownPalette = 0;
	_offsetTable = 0;
	_prefetchedFrame = 0xFFFE;
	_looping = false;
	_buffer = data;

	// I like these Streams .... =)
	Common::MemoryReadStream datastream(data, size);

	datastream.read(&_wsaHeader, sizeof(_wsaHeader));

	// check for version
	if (!_wsaHeader._type) {
		error("loading a WSA version 1 with the WSA version 2 loader");
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

	_offsetTable = new uint32[_wsaHeader._numFrames + 2];
	assert(_offsetTable);

	// loads the offset table
	for (uint32 tmp = 0; tmp < (uint32)_wsaHeader._numFrames + 2; ++tmp) {
		_offsetTable[tmp] = datastream.readUint32LE() + offsetAdd;
	}

	if (offsetAdd) {
		uint8* palbuffer = new uint8[offsetAdd];
		assert(palbuffer);

		datastream.read(palbuffer, offsetAdd);

		_ownPalette = new Palette(palbuffer, offsetAdd);
		assert(_ownPalette);
	}

	if (_offsetTable[_wsaHeader._numFrames + 1] - offsetAdd) {
		++_wsaHeader._numFrames;
		_looping = true;
	}

	_frameCount = _wsaHeader._numFrames;
}

WSAMovieV2::~WSAMovieV2() {
	delete [] _buffer;
	delete [] _offsetTable;
	delete [] _currentFrame;
	delete _ownPalette;
}

const uint8* WSAMovieV2::loadFrame(uint16 frame, uint16* width, uint16* height) {
	if (width) *width = _wsaHeader._width;
	if (height) *height = _wsaHeader._height;

	if (frame == _prefetchedFrame) {
		return _currentFrame;
	} else {
		if (!_currentFrame) {
			_currentFrame = new uint8[_wsaHeader._width * _wsaHeader._height];
			assert(_currentFrame);
			memset(_currentFrame, 0, sizeof(uint8) * _wsaHeader._width * _wsaHeader._height);
		}

		if (frame >= _wsaHeader._numFrames)
			return 0;

		uint8* frameData = 0;
		static uint8 image40[64000]; // I think this will crash on Plam OS :)
		memset(image40, 0, ARRAYSIZE(image40));

		if (frame == _prefetchedFrame + 1) {
			frameData = _buffer + _offsetTable[frame];
			Compression::decode80(frameData, image40);
			Compression::decode40(image40, _currentFrame);
		} else {
			if (_background) {
				setImageBackground(_background, _backWidth, _backHeight);
			} else {
				memset(_currentFrame, 0, sizeof(uint8) * _wsaHeader._width * _wsaHeader._height);
			}

			for (uint32 i = 0; i <= frame; ++i)
			{
				frameData = _buffer + _offsetTable[i];
				Compression::decode80(frameData, image40);
				Compression::decode40(image40, _currentFrame);
			}
		}

		_prefetchedFrame = frame;
		return _currentFrame;
	}

	return 0;
}

void WSAMovieV2::renderFrame(uint8* plane, uint16 planepitch, uint16 planeheight, uint16 frame) {
	if (!loadFrame(frame, 0, 0))
		return;

	uint8* src = _currentFrame;
	uint8* dst = &plane[_wsaHeader._yPos * planepitch + _wsaHeader._xPos];
	uint32 copysize = planepitch - _wsaHeader._xPos;

	if (copysize > _wsaHeader._width)
		copysize = _wsaHeader._width;

	if (_transparency == -1) {
		for (uint16 y_ = 0; y_ < _wsaHeader._height && _wsaHeader._yPos + y_ < planeheight; ++y_) {
			memcpy(dst, src, copysize * sizeof(uint8));
			dst += planepitch;
			src += _wsaHeader._width;
		}
	} else {
		for (uint16 yadd = 0; yadd < _wsaHeader._height; ++yadd) {
			for (uint16 xadd = 0; xadd < copysize; ++xadd) {
				if (*src == _transparency) {
					++dst;
					++src;
				} else {
					*dst++ = *src++;
				}
			}

			src += _wsaHeader._width - copysize;
			dst += planepitch - copysize;
		}
	}
}

void WSAMovieV2::setImageBackground(uint8* plane, uint16 planepitch, uint16 height) {
	assert(plane);

	_background = plane;
	_backWidth = planepitch; _backHeight = height;

	if (!_currentFrame) {
		_currentFrame = new uint8[_wsaHeader._width * _wsaHeader._height];
		assert(_currentFrame);
	}

	memset(_currentFrame, 0, sizeof(uint8) * _wsaHeader._width * _wsaHeader._height);

	uint8* src = &plane[_wsaHeader._yPos * planepitch + _wsaHeader._xPos];
	uint8* dst = _currentFrame;
	uint32 copysize = planepitch - _wsaHeader._xPos;

	if (copysize > _wsaHeader._width)
		copysize = _wsaHeader._width;

	// now copy the rect of the plane
	for (uint16 y_ = 0; y_ < _wsaHeader._height && _wsaHeader._yPos + y_ < height; ++y_) {
		memcpy(dst, src, copysize * sizeof(uint8));
		dst += _wsaHeader._width;
		src += planepitch;
	}

	for (uint16 y_ = 0; y_ < _wsaHeader._height && _wsaHeader._yPos + y_ < height; ++y_) {
		for (uint16 x = 0; x < _wsaHeader._width; ++x) {
			_currentFrame[y_ * _wsaHeader._width + x] ^= 0;
		}
	}

	_prefetchedFrame = 0xFFFE;
}
} // end of namespace Kyra

