/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

class Font : public PoolObject<Font> {
public:
	Font();
	~Font();

	static int32 getStaticTag() { return MKTAG('F', 'O', 'N', 'T'); }

	void load(const Common::String &filename, Common::SeekableReadStream *data);

	const Common::String &getFilename() const { return _filename; }
	int32 getKernedHeight() const { return _kernedHeight; }
	int32 getBaseOffsetY() const { return _baseOffsetY; }
	int32 getCharBitmapWidth(unsigned char c) const { return _charHeaders[getCharIndex(c)].bitmapWidth; }
	int32 getCharBitmapHeight(unsigned char c) const { return _charHeaders[getCharIndex(c)].bitmapHeight; }
	int32 getCharKernedWidth(unsigned char c) const { return _charHeaders[getCharIndex(c)].kernedWidth; }
	int32 getCharStartingCol(unsigned char c) const { return _charHeaders[getCharIndex(c)].startingCol; }
	int32 getCharStartingLine(unsigned char c) const { return _charHeaders[getCharIndex(c)].startingLine; }
	int32 getCharOffset(unsigned char c) const { return _charHeaders[getCharIndex(c)].offset; }
	const byte *getCharData(unsigned char c) const { return _fontData + (_charHeaders[getCharIndex(c)].offset); }

	const byte *getFontData() const { return _fontData; }
	uint32 getDataSize() const { return _dataSize; }

	int getKernedStringLength(const Common::String &text) const;
	int getBitmapStringLength(const Common::String &text) const;
	int getStringHeight(const Common::String &text) const;

	const void *getUserData() const { return _userData; }
	void setUserData(void *data) { _userData = data; }

	void saveState(SaveGame *state) const;
	void restoreState(SaveGame *state);

	static const uint8 emerFont[][13];
private:

	uint16 getCharIndex(unsigned char c) const;
	struct CharHeader {
		int32 offset;
		int8  kernedWidth;
		int8  startingCol;
		int8  startingLine;
		int32 bitmapWidth;
		int32 bitmapHeight;
	};

	uint32 _numChars;
	uint32 _dataSize;
	uint32 _kernedHeight, _baseOffsetY;
	uint32 _firstChar, _lastChar;
	uint16 *_charIndex;
	CharHeader *_charHeaders;
	byte *_fontData;
	Common::String _filename;
	void *_userData;
};

} // end of namespace Grim

#endif
