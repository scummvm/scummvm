/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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
 */

#ifndef GRIM_FONT_H
#define GRIM_FONT_H

#include "engines/grim/pool.h"

namespace Common {
class SeekableReadStream;
}

namespace Grim {

class SaveGame;

class Font : public PoolObject<Font, MKTAG('F', 'O', 'N', 'T')> {
public:
	Font(const Common::String &filename, Common::SeekableReadStream *data);
	Font();
	~Font();
	void load(const Common::String &filename, Common::SeekableReadStream *data);

	const Common::String &getFilename() const { return _filename; }
	int32 getHeight() const { return _height; }
	int32 getBaseOffsetY() const { return _baseOffsetY; }
	int32 getCharDataWidth(unsigned char c) const { return _charHeaders[getCharIndex(c)].dataWidth; }
	int32 getCharDataHeight(unsigned char c) const { return _charHeaders[getCharIndex(c)].dataHeight; }
	int32 getCharWidth(unsigned char c) const { return _charHeaders[getCharIndex(c)].width; }
	int32 getCharStartingCol(unsigned char c) const { return _charHeaders[getCharIndex(c)].startingCol; }
	int32 getCharStartingLine(unsigned char c) const { return _charHeaders[getCharIndex(c)].startingLine; }
	int32 getCharOffset(unsigned char c) const { return _charHeaders[getCharIndex(c)].offset; }
	const byte *getCharData(unsigned char c) const { return _fontData + (_charHeaders[getCharIndex(c)].offset); }

	const byte *getFontData() const { return _fontData; }
	uint32 getDataSize() const { return _dataSize; }

	int getStringLength(const Common::String &text) const;

	void *getUserData() { return _userData; }
	void setUserData(void *data) { _userData = data; }

	void saveState(SaveGame *state) const;
	void restoreState(SaveGame *state);

	static const uint8 emerFont[][13];
private:

	uint16 getCharIndex(unsigned char c) const;
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
	void *_userData;
};

} // end of namespace Grim

#endif
