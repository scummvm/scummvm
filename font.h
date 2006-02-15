// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#ifndef FONT_H
#define FONT_H

#include "bits.h"
#include "resource.h"

#include <list>

class Font : public Resource {
public:
	Font(const char *filename, const char *data, int len);
	~Font();

	int32 getCharWidth(unsigned char c) { return _charHeaders[getCharIndex(c)].width; }
	int32 getCharHeight(unsigned char c) { return _charHeaders[getCharIndex(c)].height; }
	int32 getCharLogicalWidth(unsigned char c) { return _charHeaders[getCharIndex(c)].logicalWidth; }
	int32 getCharStartingCol(unsigned char c) { return _charHeaders[getCharIndex(c)].startingCol; }
	int32 getCharStartingLine(unsigned char c) { return _charHeaders[getCharIndex(c)].startingLine; }
	const byte *getCharData(unsigned char c) { return _fontData + (_charHeaders[getCharIndex(c)].offset); }

	static const uint8 emerFont[][13];
private:

	uint16 getCharIndex(unsigned char c);
	struct CharHeader {
		int32 offset;
		int32 unknown;
		int8  logicalWidth;
		int8  startingCol;
		int8  startingLine;
		int32 width;
		int32 height;
	};

	uint32 _numChars;
	uint32 _dataSize;
	uint32 _maxCharWidth, _maxCharHeight;
	uint32 _unknownHeader1, _unknownHeader2;
	uint32 _firstChar, _lastChar;
	uint16 *_charIndex;
	CharHeader *_charHeaders;
	byte *_fontData;
};

#endif
