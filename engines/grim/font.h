/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#ifndef GRIM_FONT_H
#define GRIM_FONT_H

#include "engines/grim/resource.h"

namespace Grim {

class Font : public Resource {
public:
	Font(const char *filename, const char *data, int len);
	~Font();

	Common::String getFilename() { return _filename; }
	int32 getHeight() { return _height; }
	int32 getBaseOffsetY() { return _baseOffsetY; }
	int32 getCharDataWidth(unsigned char c) { return _charHeaders[getCharIndex(c)].dataWidth; }
	int32 getCharDataHeight(unsigned char c) { return _charHeaders[getCharIndex(c)].dataHeight; }
	int32 getCharWidth(unsigned char c) { return _charHeaders[getCharIndex(c)].width; }
	int32 getCharStartingCol(unsigned char c) { return _charHeaders[getCharIndex(c)].startingCol; }
	int32 getCharStartingLine(unsigned char c) { return _charHeaders[getCharIndex(c)].startingLine; }
	const byte *getCharData(unsigned char c) { return _fontData + (_charHeaders[getCharIndex(c)].offset); }

	static const uint8 emerFont[][13];
private:

	uint16 getCharIndex(unsigned char c);
	struct CharHeader {
		int32 offset;
		int8  width;
		int8  startingCol;
		int8  startingLine;
		int32 dataWidth;
		int32 dataHeight;
	};

	uint32 _numChars;
	uint32 _dataSize;
	uint32 _height, _baseOffsetY;
	uint32 _firstChar, _lastChar;
	uint16 *_charIndex;
	CharHeader *_charHeaders;
	byte *_fontData;
	Common::String _filename;
};

} // end of namespace Grim

#endif
